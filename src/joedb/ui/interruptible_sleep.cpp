#include "joedb/ui/interruptible_sleep.h"
#include "joedb/Signal.h"

#include <thread>
#include <algorithm>

namespace joedb
{
 bool interruptible_sleep(std::chrono::milliseconds duration)
 {
  const std::chrono::milliseconds step{1000};

  while (duration.count() > 0)
  {
   const std::chrono::milliseconds actual_step = std::min(step, duration);
   std::this_thread::sleep_for(actual_step);
   if (Signal::get_signal() == SIGINT || Signal::get_signal() == SIGTERM)
    return false;
   duration -= actual_step;
  }

  return true;
 }
}
