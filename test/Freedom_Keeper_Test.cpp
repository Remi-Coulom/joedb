#include "joedb/Freedom_Keeper.h"
#include "gtest/gtest.h"

namespace joedb
{
 #ifndef NDEBUG
 ////////////////////////////////////////////////////////////////////////////
 TEST(Freedom_Keeper, exceptions)
 ////////////////////////////////////////////////////////////////////////////
 {
  List_Freedom_Keeper fk;

  EXPECT_ANY_THROW(fk.use(Record_Id{0}));
  EXPECT_ANY_THROW(fk.use(Record_Id{1}));
  EXPECT_ANY_THROW(fk.use(Record_Id{2}));

  const Record_Id i = fk.push_back();
  fk.use(i);
  EXPECT_ANY_THROW(fk.use(i));

  fk.free(i);
  EXPECT_ANY_THROW(fk.free(i));
  EXPECT_ANY_THROW(fk.free(Record_Id{0}));
  EXPECT_ANY_THROW(fk.free(Record_Id{10}));
 }
 #endif

 template<typename T>
 class Freedom_Keeper_Test: public ::testing::Test
 {
  public:
   T fk;
 };

 using fk_types = ::testing::Types
 <
  List_Freedom_Keeper,
  Compact_Freedom_Keeper
 >;

 TYPED_TEST_SUITE(Freedom_Keeper_Test, fk_types,);

 #ifndef NDEBUG
 ////////////////////////////////////////////////////////////////////////////
 TYPED_TEST(Freedom_Keeper_Test, errors)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto &fk = this->fk;

  EXPECT_ANY_THROW(fk.use(Record_Id{1}));
  EXPECT_ANY_THROW(fk.use(Record_Id{2}));
 }
 #endif

 ////////////////////////////////////////////////////////////////////////////
 TYPED_TEST(Freedom_Keeper_Test, basic)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto &fk = this->fk;
  EXPECT_TRUE(fk.is_compact());

  EXPECT_EQ(0, fk.size());
  EXPECT_EQ(Record_Id{0}, fk.push_back());
  EXPECT_TRUE(fk.is_free(Record_Id{0}));
  EXPECT_EQ(1, fk.size());
  fk.use(Record_Id{0});
  EXPECT_FALSE(fk.is_free(Record_Id{0}));
  EXPECT_TRUE(fk.is_compact());

  fk.use(fk.push_back()); // 1
  fk.use(fk.push_back()); // 2
  fk.use(fk.push_back()); // 3
  fk.use(fk.push_back()); // 4
  fk.use(fk.push_back()); // 5
  fk.use(fk.push_back()); // 6

  Record_Id pushed = fk.push_back();
  EXPECT_EQ(Record_Id{7}, pushed);

  EXPECT_EQ(8, fk.size());
  for (Record_Id i{0}; i <= Record_Id{6}; ++i)
  {
   EXPECT_FALSE(fk.is_free(i));
   EXPECT_TRUE(fk.is_used(i));
  }

  fk.free(Record_Id{3});
  EXPECT_FALSE(fk.is_compact());

  EXPECT_EQ(8, fk.size());

  EXPECT_TRUE(fk.is_free(Record_Id{7}));
  EXPECT_TRUE(fk.is_free(Record_Id{3}));
  EXPECT_FALSE(fk.is_used(Record_Id{3}));

  for (int i = 0; i <= 6; i++)
   if (i != 3)
   {
    EXPECT_FALSE(fk.is_free(Record_Id{i}));
    EXPECT_TRUE(fk.is_used(Record_Id{i}));
   }
 }

 ////////////////////////////////////////////////////////////////////////////
 TYPED_TEST(Freedom_Keeper_Test, previous_next)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto &fk = this->fk;

  EXPECT_EQ(Record_Id{-2}, fk.get_first_used());
  EXPECT_EQ(Record_Id{-1}, fk.get_first_free());

  EXPECT_EQ(Record_Id{-2}, fk.get_next(Record_Id{-2}));
  EXPECT_EQ(Record_Id{-2}, fk.get_previous(Record_Id{-2}));

  EXPECT_EQ(Record_Id{-1}, fk.get_next(Record_Id{-1}));
  EXPECT_EQ(Record_Id{-1}, fk.get_previous(Record_Id{-1}));

  fk.use(fk.push_back());
  EXPECT_EQ(1, fk.size());
  EXPECT_EQ(Record_Id{ 0}, fk.get_first_used());
  EXPECT_EQ(Record_Id{-2}, fk.get_previous(Record_Id{0}));
  EXPECT_EQ(Record_Id{-2}, fk.get_next(Record_Id{0}));
  EXPECT_EQ(Record_Id{ 0}, fk.get_previous(Record_Id{-2}));
  EXPECT_EQ(Record_Id{ 0}, fk.get_next(Record_Id{-2}));
  EXPECT_EQ(Record_Id{-1}, fk.get_next(Record_Id{-1}));
  EXPECT_EQ(Record_Id{-1}, fk.get_previous(Record_Id{-1}));

  {
   const auto i = fk.push_back();
   EXPECT_EQ(Record_Id{ 1}, i);
   EXPECT_EQ(Record_Id{ 1}, fk.get_used_count());
   EXPECT_EQ(Record_Id{ 2}, fk.get_size());
   EXPECT_EQ(Record_Id{-1}, fk.get_next(i));
   EXPECT_EQ(Record_Id{-1}, fk.get_previous(i));
   EXPECT_EQ(i, fk.get_next(Record_Id{-1}));
   EXPECT_EQ(i, fk.get_previous(Record_Id{-1}));

   EXPECT_EQ(Record_Id{-2}, fk.get_previous(Record_Id{0}));
   EXPECT_EQ(Record_Id{-2}, fk.get_next(Record_Id{0}));
   EXPECT_EQ(Record_Id{ 0}, fk.get_previous(Record_Id{-2}));
   EXPECT_EQ(Record_Id{ 0}, fk.get_next(Record_Id{-2}));

   fk.use(i);
   EXPECT_EQ(Record_Id{-1}, fk.get_next(Record_Id{-1}));
   EXPECT_EQ(Record_Id{-1}, fk.get_previous(Record_Id{-1}));

   EXPECT_EQ(Record_Id{ 0}, fk.get_previous(Record_Id{1}));
   EXPECT_EQ(Record_Id{-2}, fk.get_next(Record_Id{1}));

   EXPECT_EQ(Record_Id{ 1}, fk.get_previous(Record_Id{-2}));
   EXPECT_EQ(Record_Id{ 1}, fk.get_next(Record_Id{0}));
  }
  EXPECT_EQ(Record_Id{ 0}, fk.get_next(Record_Id{-2}));
  EXPECT_EQ(Record_Id{ 1}, fk.get_next(Record_Id{0}));
  EXPECT_EQ(Record_Id{-2}, fk.get_next(Record_Id{1}));

  fk.use(fk.push_back());

  EXPECT_EQ(Record_Id{-1}, fk.get_first_free());

  fk.push_back();

  EXPECT_EQ(Record_Id{3}, fk.get_first_free());

  fk.push_back();
  fk.push_back();

  EXPECT_EQ(Record_Id{ 1}, fk.get_next(Record_Id{0}));
  EXPECT_EQ(Record_Id{ 2}, fk.get_next(Record_Id{1}));
  EXPECT_EQ(Record_Id{-2}, fk.get_next(Record_Id{2}));
  EXPECT_EQ(Record_Id{ 0}, fk.get_next(Record_Id{-2}));

  EXPECT_EQ(Record_Id{-2}, fk.get_previous(Record_Id{0}));
  EXPECT_EQ(Record_Id{ 0}, fk.get_previous(Record_Id{1}));
  EXPECT_EQ(Record_Id{ 1}, fk.get_previous(Record_Id{2}));
  EXPECT_EQ(Record_Id{ 2}, fk.get_previous(Record_Id{-2}));
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Freedom_Keeper, compactness)
 ////////////////////////////////////////////////////////////////////////////
 {
  Compact_Freedom_Keeper fk;
  EXPECT_TRUE(fk.is_compact());

  for (int i = 8; --i >= 0;)
   fk.use(fk.push_back());

  EXPECT_TRUE(fk.is_compact());
  fk.free(Record_Id{7});
  EXPECT_TRUE(fk.is_compact());
  fk.free(Record_Id{4});
  EXPECT_FALSE(fk.is_compact());
 }
}
