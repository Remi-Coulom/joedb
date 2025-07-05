#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include "joedb/asio/Server.h"
#include "joedb/rpc/Procedures.h"

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

     boost::asio::awaitable<void> handshake()
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

     boost::asio::awaitable<void> procedure()
     {
      co_await read_buffer(1, 16);

      const int64_t id = buffer.read<int64_t>();
      const int64_t until = buffer.read<int64_t>();

      if (id < 0 || size_t(id) >= get_server().procedures.size())
       throw Exception("bad procedure id");

      // TODO: execute procedure

      buffer.index = 1;
      buffer.write<int64_t>(until);

      co_await write_buffer();
     }

    public:
     Session
     (
      Server &server,
      boost::asio::local::stream_protocol::socket &&socket
     ):
      joedb::asio::Server::Session(server, std::move(socket))
     {
     }

     boost::asio::awaitable<void> run() override
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

   std::unique_ptr<joedb::asio::Server::Session> new_session
   (
    boost::asio::local::stream_protocol::socket &&socket
   ) override
   {
    return std::make_unique<Session>(*this, std::move(socket));
   }

  public:
   Server
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
