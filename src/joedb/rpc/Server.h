#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include "joedb/asio/Server.h"
#include "joedb/rpc/Procedure.h"

namespace joedb::rpc
{
 /// RPC Server
 ///
 /// @ingroup RPC
 class Server: public joedb::asio::Server
 {
  private:
   std::vector<std::reference_wrapper<Procedure>> &procedures;

  protected:
   class Session: public joedb::asio::Server::Session
   {
    private:
     Server &get_server() {return *(Server *)&server;}

     boost::asio::awaitable<void> handshake()
     {
      co_await read_buffer(0, 32);

      // TODO: check correct hash

      buffer.index = 1;
      buffer.write<int64_t>(id);

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
      while (true)
      {
       co_await read_buffer(0, 1);

       if (server.get_log_level() > 2)
        log(std::string("received command: ") + buffer.data[0]);

       switch (buffer.data[0])
       {
        case 'H': co_await handshake(); break;
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
    std::vector<std::reference_wrapper<Procedure>> &procedures
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
