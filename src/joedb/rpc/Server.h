#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include "joedb/asio/Server.h"
#include "joedb/rpc/Procedures.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Writable_Journal.h"

#include <algorithm>

namespace joedb::rpc
{
 /// RPC Server
 ///
 /// @ingroup RPC
 class Server: public joedb::asio::Server
 {
  private:
   const Procedures &procedures;

  protected:
   class Session: public joedb::asio::Server::Session
   {
    private:
     Server &get_server() {return *(Server *)&server;}

     ////////////////////////////////////////////////////////////////////////
     boost::asio::awaitable<void> handshake()
     ////////////////////////////////////////////////////////////////////////
     {
      co_await read_buffer(0, 32);
      const auto hash = buffer.read<SHA_256::Hash>();
      const bool correct_hash = hash == get_server().procedures.get_hash();

      buffer.index = 0;
      buffer.write<char>(correct_hash ? 'H': 'h');
      buffer.write<int64_t>(id);

      if (get_server().log_level > 2)
       log(correct_hash ? "correct hash" : "incorrect hash");

      co_await write_buffer();
     }

     ////////////////////////////////////////////////////////////////////////
     boost::asio::awaitable<void> procedure()
     ////////////////////////////////////////////////////////////////////////
     {
      co_await read_buffer(1, 16);

      const int64_t id = buffer.read<int64_t>();
      const int64_t until = buffer.read<int64_t>();

      if (id < 0 || size_t(id) >= get_server().procedures.size())
       throw Exception("bad procedure id");

      if (get_server().log_level > 2)
      {
       log
       (
        "procedure[" + std::to_string(id) +
        "]: " + get_server().procedures.get_names()[id]
       );
      }

      auto &procedure = *get_server().procedures.get_procedures()[id];
      Memory_File file;
      const std::string &prolog = procedure.get_prolog();
      file.write_data(prolog.data(), prolog.size());

      int64_t remaining = until - file.get_position();
      log("remaining = " + std::to_string(remaining));

      while (remaining > 0)
      {
       const size_t n = co_await read_buffer
       (
        0,
        std::min(remaining, int64_t(buffer.size))
       );

       file.write_data(buffer.data, n);

       remaining -= n;
      }

      file.flush();

      {
       Writable_Journal journal
       (
        Journal_Construction_Lock{file, Recovery::ignore_header}
       );
       journal.soft_checkpoint();
      }

      buffer.index = 0;

      try
      {
       procedure.execute(file);
       buffer.write<char>('P');
       buffer.write<int64_t>(until);

       // TODO: send reply data
      }
      catch (const std::exception &e)
      {
       // TODO: transaction roll-back

       const std::string message(e.what());

       if (get_server().log_level > 2)
        log("error: " + message);

       const size_t n = std::min(message.size(), buffer.size - 9);
       buffer.write<char>('p');
       buffer.write<int64_t>(int64_t(n));
       std::strncpy(buffer.data + buffer.index, message.data(), n);
       buffer.index += n;
      }

      co_await write_buffer();
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
      co_await handshake();

      while (true)
      {
       co_await read_buffer(0, 1);

       if (server.get_log_level() > 2)
        log(std::string("received command: ") + buffer.data[0]);

       switch (buffer.data[0])
       {
        case 'P': co_await procedure(); break;
        default: co_return; break;
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
    const Procedures &procedures
   ):
    joedb::asio::Server
    (
     logger,
     log_level,
     thread_count,
     std::move(endpoint_path)
    ),
    procedures(procedures)
   {
   }
 };
}

#endif
