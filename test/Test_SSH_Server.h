#ifndef joedb_Test_SSH_Server_declared
#define joedb_Test_SSH_Server_declared

#include "joedb/error/assert.h"
#include <libssh/server.h>
#include <libssh/callbacks.h>
#include <boost/asio/ip/tcp.hpp>
#include <atomic>
#include <memory>
#include <thread>
#include <libssh/sftp.h>

namespace joedb
{
 class Test_SSH_Key
 {
  private:
   ssh_key key = nullptr;
   char *text = nullptr;
  public:
   Test_SSH_Key()
   {
    JOEDB_RELEASE_ASSERT(ssh_pki_generate(SSH_KEYTYPE_ED25519, 0, &key) == SSH_OK);
    JOEDB_RELEASE_ASSERT(ssh_pki_export_privkey_base64(key, nullptr, nullptr, nullptr, &text) == SSH_OK);
   }
   Test_SSH_Key(const Test_SSH_Key &) = delete;
   Test_SSH_Key &operator=(const Test_SSH_Key &) = delete;
   const char *get_text() const {return text;}
   ~Test_SSH_Key()
   {
    ssh_string_free_char(text);
    ssh_key_free(key);
   }
 };

 class Test_SSH_Server
 {
  private:
   boost::asio::io_context context;
   boost::asio::ip::tcp::acceptor acceptor
    {context, {boost::asio::ip::address_v4::loopback(), 0}};
   std::unique_ptr<ssh_bind_struct, decltype(&ssh_bind_free)> bind{ssh_bind_new(), ssh_bind_free};
   std::atomic<bool> stopping{false};
   const bool allow_sftp;
   std::thread thread;

   struct Session
   {
    std::unique_ptr<ssh_session_struct, decltype(&ssh_free)> session{ssh_new(), ssh_free};
    std::unique_ptr<ssh_event_struct, decltype(&ssh_event_free)> event{ssh_event_new(), ssh_event_free};
    ssh_channel channel = nullptr;
    ssh_server_callbacks_struct callbacks{};
    ssh_channel_callbacks_struct channel_callbacks{};
    const bool allow_sftp;
    bool sftp_requested = false;

    static int open_channel(ssh_session, ssh_message message, void *data)
    {
     auto &self = *static_cast<Session *>(data);
     if (self.allow_sftp && ssh_message_type(message) == SSH_REQUEST_CHANNEL &&
         ssh_message_subtype(message) == SSH_CHANNEL_REQUEST_SUBSYSTEM &&
         std::string(ssh_message_channel_request_subsystem(message)) == "sftp")
     {
      ssh_message_channel_request_reply_success(message);
      self.sftp_requested = true;
      return 0;
     }
     if (ssh_message_type(message) != SSH_REQUEST_CHANNEL_OPEN ||
         ssh_message_subtype(message) != SSH_CHANNEL_DIRECT_TCPIP)
      return 1;
     self.channel = ssh_message_channel_request_open_reply_accept(message);
     if (self.channel)
      ssh_set_channel_callbacks(self.channel, &self.channel_callbacks);
     return 0;
    }

    explicit Session(bool allow_sftp): allow_sftp(allow_sftp)
    {
     JOEDB_RELEASE_ASSERT(session && event);
     callbacks.userdata = this;
     callbacks.auth_pubkey_function = [](ssh_session, const char *, ssh_key, char state, void *) -> int
     {
      return state == SSH_PUBLICKEY_STATE_NONE || state == SSH_PUBLICKEY_STATE_VALID
       ? SSH_AUTH_SUCCESS : SSH_AUTH_DENIED;
     };
     callbacks.service_request_function = [](ssh_session, const char *, void *) {return 0;};
     callbacks.channel_open_request_session_function = [](ssh_session session, void *data) -> ssh_channel
     {
      auto &self = *static_cast<Session *>(data);
      if (!self.allow_sftp)
       return nullptr;
      self.channel = ssh_channel_new(session);
      return self.channel;
     };
     ssh_callbacks_init(&callbacks);
     ssh_set_server_callbacks(session.get(), &callbacks);
     ssh_set_auth_methods(session.get(), SSH_AUTH_METHOD_PUBLICKEY);
     ssh_set_message_callback(session.get(), open_channel, this);
     channel_callbacks.channel_data_function = [](ssh_session, ssh_channel channel,
      void *data, uint32_t size, int, void *)
     {
      const int written = ssh_channel_write(channel, data, size);
      ssh_channel_send_eof(channel);
      return written;
     };
     ssh_callbacks_init(&channel_callbacks);
     const long timeout = 2;
     ssh_options_set(session.get(), SSH_OPTIONS_TIMEOUT, &timeout);
    }

    static void append_number(std::string &packet, uint32_t number)
    {
     for (int shift: {24, 16, 8, 0})
      packet += char(number >> shift);
    }
    static void append_string(std::string &packet, const std::string &text)
    {
     append_number(packet, uint32_t(text.size()));
     packet += text;
    }
    static uint64_t number(const std::string &packet, size_t start, size_t count)
    {
     uint64_t result = 0;
     for (size_t i = 0; i < count; ++i)
      result = (result << 8) | uint8_t(packet.at(start + i));
     return result;
    }
    bool read_bytes(std::string &data, size_t size)
    {
     data.resize(size);
     size_t done = 0;
     while (done < size)
     {
      const int count = ssh_channel_read_timeout(channel, data.data() + done,
       uint32_t(size - done), 0, 2000);
      if (count <= 0)
       return false;
      done += size_t(count);
     }
     return true;
    }

    // A tiny SFTP v3 peer with two read-only virtual files, no filesystem access.
    void serve_sftp()
    {
     std::string length;
     while (read_bytes(length, 4))
     {
      const size_t size = size_t(number(length, 0, 4));
      std::string request;
      if (size == 0 || size > 65536 || !read_bytes(request, size))
       return;
      std::string response;
      if (uint8_t(request[0]) == SSH_FXP_INIT)
      {
       response += char(SSH_FXP_VERSION);
       append_number(response, 3);
      }
      else
      {
       if (request.size() < 5)
        return;
       const uint32_t id = uint32_t(number(request, 1, 4));
       const auto status = [&](uint32_t code)
       {
        response = char(SSH_FXP_STATUS);
        append_number(response, id);
        append_number(response, code);
        append_string(response, "test peer");
        append_string(response, "");
       };
       if (uint8_t(request[0]) == SSH_FXP_OPEN)
       {
        if (request.size() < 9)
         return;
        const std::string name = request.substr(9, size_t(number(request, 5, 4)));
        if (name == "payload" || name == "broken")
        {
         response += char(SSH_FXP_HANDLE);
         append_number(response, id);
         append_string(response, name);
        }
        else
         status(SSH_FX_NO_SUCH_FILE);
       }
       else if (uint8_t(request[0]) == SSH_FXP_READ)
       {
        if (request.size() < 9)
         return;
        const size_t handle_size = size_t(number(request, 5, 4));
        if (request.size() < 21 + handle_size)
         return;
        const std::string handle = request.substr(9, handle_size);
        const uint64_t offset = number(request, 9 + handle_size, 8);
        const size_t asked = size_t(number(request, 17 + handle_size, 4));
        const std::string body = "0123456789";
        if (handle == "broken")
         status(SSH_FX_FAILURE);
        else if (offset >= body.size())
         status(SSH_FX_EOF);
        else
        {
         response += char(SSH_FXP_DATA);
         append_number(response, id);
         append_string(response, body.substr(size_t(offset), std::min<size_t>(asked, 3)));
        }
       }
       else if (uint8_t(request[0]) == SSH_FXP_CLOSE)
        status(SSH_FX_OK);
       else
        status(SSH_FX_OP_UNSUPPORTED);
      }
      std::string packet;
      append_number(packet, uint32_t(response.size()));
      packet += response;
      size_t sent = 0;
      while (sent < packet.size())
      {
       const int count = ssh_channel_write(channel, packet.data() + sent, uint32_t(packet.size() - sent));
       if (count <= 0)
        return;
       sent += size_t(count);
      }
     }
    }

    ~Session()
    {
     ssh_event_remove_session(event.get(), session.get());
     if (channel)
      ssh_channel_free(channel);
     ssh_disconnect(session.get());
    }
   };

  public:
   explicit Test_SSH_Server(bool allow_sftp = false): allow_sftp(allow_sftp)
   {
    JOEDB_RELEASE_ASSERT(bind);
    ssh_key host_key = nullptr;
    JOEDB_RELEASE_ASSERT(ssh_pki_generate(SSH_KEYTYPE_ED25519, 0, &host_key) == SSH_OK);
    // The bind owns the imported host key.
    JOEDB_RELEASE_ASSERT(ssh_bind_options_set(bind.get(), SSH_BIND_OPTIONS_IMPORT_KEY, host_key) == SSH_OK);
    acceptor.async_accept([this](const boost::system::error_code &error,
     boost::asio::ip::tcp::socket socket)
    {
     if (error)
      return;
     Session client(this->allow_sftp);
     if (ssh_bind_accept_fd(bind.get(), client.session.get(), socket.release()) != SSH_OK ||
         ssh_handle_key_exchange(client.session.get()) != SSH_OK)
      return;
     ssh_event_add_session(client.event.get(), client.session.get());
     while (!stopping && ssh_is_connected(client.session.get()))
     {
      if (ssh_event_dopoll(client.event.get(), 100) == SSH_ERROR)
       break;
      if (client.sftp_requested)
      {
       client.serve_sftp();
       break;
      }
     }
    });
    thread = std::thread([this]() {context.run();});
   }
   unsigned port() const {return acceptor.local_endpoint().port();}
   ~Test_SSH_Server()
   {
    stopping = true;
    context.stop();
    thread.join();
   }
 };
}

#endif
