#include "gtest/gtest.h"
#include "src/gtest-all.cc"
#include "joedb/String_Logger.h"
#include "joedb/Destructor_Logger.h"

int main(int argc, char **argv)
{
 joedb::Destructor_Logger::set_logger(&joedb::String_Logger::the_logger);
 testing::InitGoogleTest(&argc, argv);
 return RUN_ALL_TESTS();
}
