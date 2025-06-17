#ifndef joedb_IO_Context_Wrapper_declared
#define joedb_IO_Context_Wrapper_declared

#include <boost/asio/io_context.hpp>

namespace joedb
{
 struct IO_Context_Wrapper
 {
  boost::asio::io_context io_context;

  IO_Context_Wrapper();
  void run();
  ~IO_Context_Wrapper();
 };
}

#endif
