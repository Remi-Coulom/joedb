#include "joedb/error/String_Logger.h"
#include "joedb/error/Destructor_Logger.h"

#include <gtest/gtest.h>

int main(int argc, char **argv)
{
#if 1
 joedb::Destructor_Logger::set_logger(&joedb::String_Logger::the_logger);
#endif
 testing::InitGoogleTest(&argc, argv);
 return RUN_ALL_TESTS();
}
