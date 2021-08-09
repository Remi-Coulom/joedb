#include "joedb/concurrency/Mutex_Lock.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
class Throwing_Mutex: public joedb::Mutex
/////////////////////////////////////////////////////////////////////////////
{
 private:
  void lock() override {}
  void unlock() override {throw std::runtime_error("youyou");}
};

/////////////////////////////////////////////////////////////////////////////
TEST(Mutex, catcher)
/////////////////////////////////////////////////////////////////////////////
{
 Throwing_Mutex mutex;
 joedb::Posthumous_Catcher catcher;

 {
  joedb::Mutex_Lock lock(mutex);
  lock.set_catcher(catcher);
 }

 try
 {
  catcher.rethrow();
  FAIL() << "catcher did not catch the exception";
 }
 catch (const std::runtime_error &e)
 {
  EXPECT_STREQ(e.what(), "youyou");
 }
}
