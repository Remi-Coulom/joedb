#ifndef joedb_ui_interruptible_sleep_declared
#define joedb_ui_interruptible_sleep_declared

#include <chrono>

namespace joedb
{
 /// @ingroup ui
 ///
 /// @return false if interrupted
 bool interruptible_sleep(std::chrono::milliseconds duration);
}

#endif
