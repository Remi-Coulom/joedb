#include "joedb/ui/Progress_Bar.h"

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 void Progress_Bar::print_progress() noexcept
 //////////////////////////////////////////////////////////////////////////
 {
  try
  {
   if (done > printed)
   {
    const auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration_since_last = now - last_print_time;
    if (done == total || duration_since_last.count() >= gap)
    {
     std::chrono::duration<double> duration_since_start = now - start;
     last_print_time = now;

     const auto done_string = std::to_string(done);
     const auto total_string = std::to_string(total);
     const auto padding = std::string(total_string.size() - done_string.size(), ' ');
     const int permil = int((1000.0 * double(done)) / double(total));

     // TODO: drop C++17, use std::format

     logger.log
     (
      padding + done_string + " / " + total_string + " " +
      std::to_string(permil / 10) + "." + std::to_string(permil % 10) +
      "% in " + std::to_string(duration_since_start.count()) + "s"
     );

     printed = done;
     gap *= 1.1;
     if (gap > 10.0)
      gap = 10.0;
    }
   }
  }
  catch (...)
  {
  }
 }

 //////////////////////////////////////////////////////////////////////////
 Progress_Bar::Progress_Bar
 //////////////////////////////////////////////////////////////////////////
 (
  const int64_t total,
  Logger &logger
 ):
  total(total),
  done(0),
  printed(0),
  gap(1.0),
  logger(logger),
  start(std::chrono::steady_clock::now()),
  last_print_time(start)
 {
 }

 //////////////////////////////////////////////////////////////////////////
 void Progress_Bar::print(const int64_t current)
 //////////////////////////////////////////////////////////////////////////
 {
  done = current;
  print_progress();
 }

 //////////////////////////////////////////////////////////////////////////
 Progress_Bar::~Progress_Bar()
 //////////////////////////////////////////////////////////////////////////
 {
  print_progress();
 }
}
