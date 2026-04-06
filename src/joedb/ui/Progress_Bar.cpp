#include "joedb/ui/Progress_Bar.h"

#include <sstream>
#include <iomanip>

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

     const double duration = duration_since_start.count();

     {
      std::ostringstream line;

      line << std::fixed << std::setprecision(1);
      const int width = int(std::to_string(total).size());
      line << std::setw(width) << done << " / " << total << ' ';
      line << std::setw(5) << 100.0f*float(done)/float(total) << '%';

      if (done > 0)
      {
       line << "; ";

       if (duration > 0.0)
       {
        double n = double(done) / duration;
        char unit = ' ';

        if (n > 1000000.0)
        {
         n /= 1000000.0;
         unit = 'M';
        }
        else if (n > 1000.0)
        {
         n /= 1000.0;
         unit = 'k';
        }

        line << std::fixed << std::setprecision(1) << n << unit << "/s";
       }

       if (done < total)
       {
        const double time_left = double(total - done) * duration / double(done);
        line << "; " << time_left << "s left";
       }
      }

      logger.log(line.str());
     }

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
