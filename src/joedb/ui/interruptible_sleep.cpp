#include "joedb/ui/interruptible_sleep.h"
#include "joedb/Signal.h"

#include <thread>
#include <algorithm>

namespace joedb
{
 bool interruptible_sleep(std::chrono::milliseconds duration)
 {
  const std::chrono::milliseconds step{1000};

  for (; Signal::get_signal() != SIGINT && duration.count() > 0;)
  {
   const std::chrono::milliseconds actual_step = std::min(step, duration);
   std::this_thread::sleep_for(actual_step);
   duration -= actual_step;
  }

  return duration.count() == 0 && Signal::get_signal() != SIGINT;
 }
}
