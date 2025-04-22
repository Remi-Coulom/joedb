#ifndef joedb_Network_Channel_declared
#define joedb_Network_Channel_declared

#include "joedb/concurrency/Connector.h"

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

namespace joedb
{
 /// @ingroup concurrency
 class Network_Channel: public Channel
 {
  protected:
   asio::io_context io_context;
   asio::ip::tcp::socket socket;

   size_t write_some(const char *data, size_t size) override;
   size_t read_some(char *data, size_t size) override;

  public:
   Network_Channel(std::string_view host, std::string_view service);
   ~Network_Channel() override;
 };

 class Network_Connector: public Connector
 {
  private:
   const std::string host;
   const std::string service;

  public:
   Network_Connector(std::string host, std::string service):
    host(std::move(host)),
    service(std::move(service))
   {
   }

   std::unique_ptr<Channel> new_channel() const override
   {
    return std::make_unique<Network_Channel>(host, service);
   }
 };
}

#endif
