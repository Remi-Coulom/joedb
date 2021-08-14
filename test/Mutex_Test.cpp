#include "joedb/concurrency/Mutex.h"

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
class Dummy_Mutex: public joedb::Mutex
/////////////////////////////////////////////////////////////////////////////
{
 private:
  void lock() override {locked = true;}
  void unlock() override {locked = false;}

 public:
  bool locked = false;
};

/////////////////////////////////////////////////////////////////////////////
TEST(Mutex, unlock_throws)
/////////////////////////////////////////////////////////////////////////////
{
 Throwing_Mutex mutex;

 try
 {
  mutex.run_while_locked([](){});
  FAIL() << "Did not catch the exception";
 }
 catch (const std::runtime_error &e)
 {
  EXPECT_STREQ(e.what(), "youyou");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Mutex, transaction_throws)
/////////////////////////////////////////////////////////////////////////////
{
 Dummy_Mutex mutex;

 try
 {
  mutex.run_while_locked([]()
  {
   throw std::runtime_error("yaya");
  });
  FAIL() << "Did not catch the exception";
 }
 catch (const std::runtime_error &e)
 {
  EXPECT_STREQ(e.what(), "yaya");
 }

 EXPECT_FALSE(mutex.locked);
}
