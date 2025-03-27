#include "joedb/error/String_Logger.h"
#include "joedb/error/Destructor_Logger.h"

#include <gtest/gtest.h>

int main(int argc, char **argv)
{
 joedb::error::Destructor_Logger::set_logger(&joedb::error::String_Logger::the_logger);
 testing::InitGoogleTest(&argc, argv);
 return RUN_ALL_TESTS();
}
