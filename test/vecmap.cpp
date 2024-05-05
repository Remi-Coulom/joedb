#include "joedb/vecmap.h"

#include "gtest/gtest.h"

namespace joedb
{
 TEST(vecmap, basic)
 {
  vecmap<int, int> map;

  map[2] = 123;

  EXPECT_EQ(map.find(0), map.end());
  EXPECT_EQ(map.find(1), map.end());
  EXPECT_NE(map.find(2), map.end());
  EXPECT_EQ(map.find(3), map.end());
  EXPECT_EQ(map[2], 123);
  EXPECT_EQ(map.find(2).value(), 123);
 }

 TEST(vecmap, arrow_operator)
 {
  struct T
  {
   int x;
   int y;
  };

  vecmap<int, T> map;

  map[2] = T{123, 234};

  EXPECT_EQ(map.find(2).value().x, 123);
  EXPECT_EQ(map.find(2).value().y, 234);
 }

 TEST(vecmap, loop)
 {
  vecmap<int, int> map;

  map[1] = 0;
  map[3] = 1;
  map[7] = 2;
 }
}
