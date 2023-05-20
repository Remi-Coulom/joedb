#include "joedb/Posthumous_Thrower.h"
#include "joedb/Logger.h"
#include "joedb/String_Logger.h"
#include "joedb/Stream_Logger.h"

#include "gtest/gtest.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class Test_Catcher: public Posthumous_Catcher
 /////////////////////////////////////////////////////////////////////////////
 {
 };

 /////////////////////////////////////////////////////////////////////////////
 class Test_Thrower: public Posthumous_Thrower
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   ~Test_Thrower()
   {
    postpone_exception("posthumous exception");
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 TEST(Posthumous_Thrower, basic)
 /////////////////////////////////////////////////////////////////////////////
 {
  std::ostringstream out;
  Stream_Logger logger(out);
  Destructor_Logger::set_logger(&logger);

  Test_Catcher catcher;
  catcher.rethrow();

  {
   Test_Thrower thrower;
   thrower.set_catcher(catcher);
  }

  EXPECT_EQ(out.str(), "joedb: posthumous exception\n");

  EXPECT_ANY_THROW(catcher.rethrow());

  Destructor_Logger::remove_logger();
  Destructor_Logger::set_logger();
  Destructor_Logger::set_logger(&joedb::String_Logger::the_logger);
 }
}
