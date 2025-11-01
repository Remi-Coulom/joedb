#ifndef joedb_io_Progress_Bar_declared
#define joedb_io_Progress_Bar_declared

#include "joedb/error/Logger.h"

#include <stdint.h>
#include <chrono>

namespace joedb
{
 /// @ingroup ui
 class Progress_Bar
 {
  private:
   const int64_t total;
   int64_t done;
   int64_t printed;
   double gap;
   Logger &logger;

   std::chrono::steady_clock::time_point start;
   std::chrono::steady_clock::time_point last_print_time;

   void print_progress() noexcept;

  public:
   Progress_Bar(int64_t total, Logger &logger);
   void print(int64_t current);
   void print_remaining(int64_t remaining) {print(total - remaining);}
   ~Progress_Bar();
 };
}

#endif
