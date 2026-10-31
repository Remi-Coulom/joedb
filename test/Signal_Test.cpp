#include "joedb/Signal.h"
#include "joedb/ui/interruptible_sleep.h"

#include "gtest/gtest.h"

namespace joedb
{
 TEST(Signal, interruptible_sleep_completion_and_interrupt)
 {
  const int previous_status = Signal::get_signal();
  Signal::set_signal(Signal::no_signal);
  EXPECT_TRUE(interruptible_sleep(std::chrono::milliseconds(0)));
  EXPECT_TRUE(interruptible_sleep(std::chrono::milliseconds(1)));
  Signal::set_signal(SIGINT);
  EXPECT_FALSE(interruptible_sleep(std::chrono::milliseconds(10000)));
  Signal::set_signal(previous_status);
 }

 TEST(Signal, captures_interrupt)
 {
  const auto previous_handler = std::signal(SIGINT, SIG_IGN);
  const int previous_status = Signal::get_signal();
  Signal::start();
  EXPECT_EQ(Signal::get_signal(), Signal::no_signal);
  EXPECT_EQ(std::raise(SIGINT), 0);
  EXPECT_EQ(Signal::get_signal(), SIGINT);
  Signal::stop();
  std::signal(SIGINT, previous_handler);
  Signal::set_signal(previous_status);
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Signal, basic)
 /////////////////////////////////////////////////////////////////////////////
 {
  Signal::start();
  Signal::stop();
  Signal::set_signal(1234);
  EXPECT_EQ(Signal::get_signal(), 1234);
 }
}
