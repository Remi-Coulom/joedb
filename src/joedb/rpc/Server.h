#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include "joedb/rpc/Procedure.h"
#include "joedb/rpc/Signature.h"
#include "joedb/rpc/get_hash.h"
#include "joedb/asio/Server.h"
#include "joedb/journal/Memory_File.h"

#include <algorithm>

namespace joedb::rpc
{
 /// RPC Server
 ///
 /// @ingroup rpc
 class Server: public joedb::asio::Server
 {
  private:
   const std::vector<Signature> &signatures;
   const std::vector<Procedure *> &procedures;

  protected:
   class Session: public joedb::asio::Server::Session
   {
    private:
     Server &get_server() {return *(Server *)&server;}

     ////////////////////////////////////////////////////////////////////////
     boost::asio::awaitable<bool> handshake()
     ////////////////////////////////////////////////////////////////////////
     {
      co_await read_buffer(0, 32);

      const SHA_256::Hash hash = buffer.read<SHA_256::Hash>();
      const bool correct_hash = hash == get_hash(get_server().signatures);

      buffer.index = 0;
      buffer.write<char>(correct_hash ? 'H': 'h');
      buffer.write<int64_t>(id);

      if (get_server().log_level > 2)
       log(correct_hash ? "correct hash" : "incorrect hash");

      co_await write_buffer();
      co_return correct_hash;
     }

     ////////////////////////////////////////////////////////////////////////
     boost::asio::awaitable<void> call()
     ////////////////////////////////////////////////////////////////////////
     {
      co_await read_buffer(1, 16);

      const size_t id = static_cast<size_t>(buffer.read<int64_t>());
      const int64_t until = buffer.read<int64_t>();

      //
      // Get procedure from id
      //
      if (id >= get_server().procedures.size())
       throw Exception("bad procedure id");

      auto &signature = get_server().signatures[id];
      auto &procedure = *get_server().procedures[id];

      if (get_server().log_level > 2)
       log("procedure[" + std::to_string(id) + "]: " + signature.name);

      //
      // Read input message into a Memory_File
      //
      Memory_File file;

      {
       std::string &data = file.get_data();
       data.reserve(size_t(until));
       data = signature.prolog;

       int64_t remaining = until - file.get_size();
       while (remaining > 0)
       {
        const size_t n = co_await read_buffer
        (
         0,
         std::min(remaining, int64_t(buffer.size))
        );
        data.append(buffer.data, n);
        remaining -= n;
       }
      }

      //
      // Execute procedure
      //
      buffer.index = 0;

      try
      {
       procedure.execute(file);
      }
      catch (const std::exception &e)
      {
       const std::string_view message(e.what());

       if (get_server().log_level > 2)
        log("error: " + std::string(message));

       const size_t n = std::min(message.size(), buffer.size - 9);
       buffer.write<char>('c');
       buffer.write<int64_t>(int64_t(n));
       std::strncpy(buffer.data + buffer.index, message.data(), n);
       buffer.index += n;
      }

      //
      // Return either an error message or the procedure output
      //
      if (buffer.index > 0)
       co_await write_buffer();
      else
      {
       buffer.index = 0;
       buffer.write<char>('C');
       buffer.write<int64_t>(file.get_size());

       size_t offset = size_t(until);
       size_t remaining = file.get_data().size() - offset;
       while (remaining + buffer.index > 0)
       {
        const size_t n = std::min(remaining, buffer.size - buffer.index);
        file.pread(buffer.data + buffer.index, n, offset);
        buffer.index += n;
        offset += n;
        remaining -= n;
        co_await write_buffer();
        buffer.index = 0;
       }
      }
     }

    public:
     ////////////////////////////////////////////////////////////////////////
     Session
     ////////////////////////////////////////////////////////////////////////
     (
      Server &server,
      boost::asio::local::stream_protocol::socket &&socket
     ):
      joedb::asio::Server::Session(server, std::move(socket))
     {
     }

     ////////////////////////////////////////////////////////////////////////
     boost::asio::awaitable<void> run() override
     ////////////////////////////////////////////////////////////////////////
     {
      if (!co_await handshake())
       co_return;

      while (true)
      {
       co_await read_buffer(0, 1);

       if (server.get_log_level() > 2)
        log(std::string("received command: ") + buffer.data[0]);

       switch (buffer.data[0])
       {
        case 'C':
         co_await call();
        break;

        case 'P':
         buffer.index = 1;
         co_await write_buffer();
        break;

        default:
         co_return;
        break;
       }
      }
     }
   };

   //////////////////////////////////////////////////////////////////////////
   std::unique_ptr<joedb::asio::Server::Session> new_session
   //////////////////////////////////////////////////////////////////////////
   (
    boost::asio::local::stream_protocol::socket &&socket
   ) override
   {
    return std::make_unique<Session>(*this, std::move(socket));
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Server
   //////////////////////////////////////////////////////////////////////////
   (
    Logger &logger,
    int log_level,
    int thread_count,
    std::string endpoint_path,
    const std::vector<Signature> &signatures,
    const std::vector<Procedure *> &procedures
   ):
    joedb::asio::Server
    (
     logger,
     log_level,
     thread_count,
     std::move(endpoint_path)
    ),
    signatures(signatures),
    procedures(procedures)
   {
   }
 };
}

#endif
