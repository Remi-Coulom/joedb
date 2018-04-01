#ifdef NDEBUG
#undef NDEBUG
#endif

#include "Freedom_Keeper.h"
#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Freedom_Keeper, exceptions)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Freedom_Keeper<> fk;

 EXPECT_ANY_THROW(fk.use(0));
 EXPECT_ANY_THROW(fk.use(1));
 EXPECT_ANY_THROW(fk.use(2));

 size_t i = fk.allocate();
 EXPECT_ANY_THROW(fk.use(i));

 fk.free(i);
 EXPECT_ANY_THROW(fk.free(i));
 EXPECT_ANY_THROW(fk.free(0));
 EXPECT_ANY_THROW(fk.free(10));
}

template<typename T>
class Freedom_Keeper_Test: public ::testing::Test
{
 public:
  T fk;
};

typedef ::testing::Types
<
 joedb::Freedom_Keeper<>,
 joedb::Compact_Freedom_Keeper
>
fk_types;

TYPED_TEST_CASE(Freedom_Keeper_Test, fk_types);

/////////////////////////////////////////////////////////////////////////////
TYPED_TEST(Freedom_Keeper_Test, errors)
/////////////////////////////////////////////////////////////////////////////
{
 auto &fk = this->fk;

 EXPECT_ANY_THROW(fk.use(1));
 EXPECT_ANY_THROW(fk.use(2));
}

/////////////////////////////////////////////////////////////////////////////
TYPED_TEST(Freedom_Keeper_Test, basic)
/////////////////////////////////////////////////////////////////////////////
{
 auto &fk = this->fk;

 EXPECT_EQ(0UL, fk.size());
 EXPECT_EQ(2UL, fk.push_back());
 EXPECT_TRUE(fk.is_free(2));
 EXPECT_EQ(1UL, fk.size());
 fk.use(2);
 EXPECT_FALSE(fk.is_free(2));

 fk.use(fk.push_back()); // 3
 fk.use(fk.push_back()); // 4
 fk.use(fk.push_back()); // 5
 fk.use(fk.push_back()); // 6
 fk.use(fk.push_back()); // 7
 fk.use(fk.push_back()); // 8

 size_t pushed = fk.push_back();
 EXPECT_EQ(9UL, pushed);

 EXPECT_EQ(8UL, fk.size());
 for (int i = 2; i <= 8; i++)
 {
  EXPECT_FALSE(fk.is_free(i));
  EXPECT_TRUE(fk.is_used(i));
 }

 fk.free(5);

 EXPECT_EQ(8UL, fk.size());

 EXPECT_TRUE(fk.is_free(9));
 EXPECT_TRUE(fk.is_free(5));
 EXPECT_FALSE(fk.is_used(5));

 for (int i = 2; i <= 8; i++)
  if (i != 5)
  {
   EXPECT_FALSE(fk.is_free(i));
   EXPECT_TRUE(fk.is_used(i));
  }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Freedom_Keeper, compactness)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Compact_Freedom_Keeper fk;
 EXPECT_TRUE(fk.is_compact());

 for (int i = 8; --i >= 0;)
  fk.use(fk.push_back());

 EXPECT_TRUE(fk.is_compact());
 fk.free(9);
 EXPECT_TRUE(fk.is_compact());
 fk.free(6);
 EXPECT_FALSE(fk.is_compact());
}
