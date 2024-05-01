#ifndef joedb_IO_Context_Wrapper_declared
#define joedb_IO_Context_Wrapper_declared

#include "joedb/concurrency/net.h"

namespace joedb
{
 struct IO_Context_Wrapper
 {
  net::io_context io_context;

  IO_Context_Wrapper();
  void run();
  ~IO_Context_Wrapper();
 };
}

#endif
