#include "joedb/Span.h"
#include "gtest/gtest.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
TEST(Span, test)
/////////////////////////////////////////////////////////////////////////////
{
 const int size = 5;

 int array[size + 1];
 joedb::Span<int> range(array, size);

 const int magic = 123;

 for (int &x: range)
  x = magic;

 for (size_t i = 0; i < range.get_size(); i++)
  EXPECT_EQ(range[i], magic);

#ifndef NDEBUG
 try
 {
  range[range.get_size()] = 0;
  FAIL() << "Should have thrown";
 }
 catch (...)
 {
 }
#endif
}
