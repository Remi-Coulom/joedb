#ifndef joedb_IO_Context_Wrapper_declared
#define joedb_IO_Context_Wrapper_declared

#include <asio/io_context.hpp>

namespace joedb
{
 /// \ingroup concurrency
 struct IO_Context_Wrapper
 {
  asio::io_context io_context;

  IO_Context_Wrapper();
  void run();
  ~IO_Context_Wrapper();
 };
}

#endif
