#ifndef joedb_Local_Channel_declared
#define joedb_Local_Channel_declared

#include "joedb/concurrency/Channel.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/local/stream_protocol.hpp>

namespace joedb
{
 /// @ingroup concurrency
 class Local_Channel: public Channel
 {
  protected:
   boost::asio::io_context io_context;
   boost::asio::local::stream_protocol::socket socket;

   size_t write_some(const char *data, size_t size) override;
   size_t read_some(char *data, size_t size) override;

  public:
   Local_Channel(const std::string &endpoint_path);
   ~Local_Channel() override;
 };
}

#endif
