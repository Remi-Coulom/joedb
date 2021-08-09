#include "joedb/Range.h"
#include "gtest/gtest.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
TEST(Range, test)
/////////////////////////////////////////////////////////////////////////////
{
 const int size = 5;

 int array[size + 1];
 joedb::Range<int> range(array, size);

 const int magic = 123;

 for (int &x: range)
  x = magic;

 for (size_t i = 0; i < range.get_size(); i++)
  EXPECT_EQ(range[i], magic);

 try
 {
  range[range.get_size()] = 0;
  FAIL() << "Should have thrown";
 }
 catch (...)
 {
 }
}
