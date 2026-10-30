#include "joedb/Freedom_Keeper.h"
#include "gtest/gtest.h"

#include <random>
#include <algorithm>
#include <limits>

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

 TEST(Freedom_Keeper, first_free_predecessor_when_all_free)
 {
  Freedom_Keeper fk;
  fk.resize(4);
  ASSERT_EQ(fk.get_first_free(), Record_Id(0));
  EXPECT_EQ(to_underlying(fk.get_previous(fk.get_first_free())), to_underlying(fk.free_list));
 }

 template<typename T>
 class Freedom_Keeper_Test: public ::testing::Test
 {
  public:
   T fk;
 };

 using fk_types = ::testing::Types
 <
  List_Freedom_Keeper,
  Freedom_Keeper
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
  EXPECT_TRUE(fk.is_dense());

  EXPECT_EQ(0, fk.size());
  EXPECT_EQ(Record_Id{0}, fk.push_back());
  EXPECT_TRUE(fk.is_free(Record_Id{0}));
  EXPECT_EQ(1, fk.size());
  fk.use(Record_Id{0});
  EXPECT_FALSE(fk.is_free(Record_Id{0}));
  EXPECT_TRUE(fk.is_dense());

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
  EXPECT_FALSE(fk.is_dense());

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
  Freedom_Keeper fk;
  EXPECT_TRUE(fk.is_dense());

  for (int i = 8; --i >= 0;)
   fk.use(fk.push_back());

  EXPECT_TRUE(fk.is_dense());
  fk.free(Record_Id{7});
  EXPECT_TRUE(fk.is_dense());
  fk.free(Record_Id{4});
  EXPECT_FALSE(fk.is_dense());
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Freedom_Keeper, vector_functions)
 ////////////////////////////////////////////////////////////////////////////
 {
  {
   Freedom_Keeper fk;
   EXPECT_TRUE(fk.is_free_vector(Record_Id{0}, 3));
   EXPECT_FALSE(fk.is_used_vector(Record_Id{0}, 3));
   fk.resize(3);
   fk.use(Record_Id{1});
   EXPECT_FALSE(fk.is_free_vector(Record_Id{0}, 3));
   EXPECT_FALSE(fk.is_used_vector(Record_Id{0}, 3));

   EXPECT_EQ(fk.get_free_record(), Record_Id{2});
   fk.use(Record_Id{2});
   EXPECT_EQ(fk.get_free_record(), Record_Id{0});
   fk.use(Record_Id{0});
   EXPECT_EQ(fk.get_free_record(), Record_Id{3});

   EXPECT_FALSE(fk.is_free_vector(Record_Id{0}, 3));
   EXPECT_TRUE(fk.is_used_vector(Record_Id{0}, 3));
   EXPECT_FALSE(fk.is_used_vector(Record_Id{0}, 4));
  }

  {
   Freedom_Keeper fk;
   fk.use(fk.push_back());
   EXPECT_TRUE(fk.is_used_vector(Record_Id{0}, 1));
   EXPECT_FALSE(fk.is_used_vector(Record_Id{0}, 2));
  }
 }
 TYPED_TEST(Freedom_Keeper_Test, used_vector_bounds)
 {
  auto &fk = this->fk;
  fk.resize(Record_Id{3});
  fk.use_vector(Record_Id{0}, 3);

  // Check both the dense representation and the list after making a hole.
  for (int pass = 0; pass < 2; ++pass)
  {
   EXPECT_TRUE(fk.is_used_vector(Record_Id{1}, 2));
   EXPECT_TRUE(fk.is_used_vector(Record_Id{3}, 0));
   EXPECT_FALSE(fk.is_used_vector(Record_Id{-1}, 0));
   EXPECT_FALSE(fk.is_used_vector(Record_Id{4}, 0));
   EXPECT_FALSE(fk.is_used_vector(Record_Id{3}, 1));
   EXPECT_FALSE(fk.is_used_vector(Record_Id{1}, 3));
   // This sum wraps to freedom_size but starts well beyond the allocation.
   EXPECT_FALSE
   (
    fk.is_used_vector
    (
     Record_Id{4096},
     std::numeric_limits<size_t>::max() - 4092
    )
   );
   if (pass == 0)
    fk.free(Record_Id{0});
  }
 }

 TYPED_TEST(Freedom_Keeper_Test, deterministic_model)
 {
  auto &fk = this->fk;
  std::vector<bool> used;
  std::mt19937 random(0x4a6f6564);
  for (size_t step = 0; step < 1200; ++step)
  {
   SCOPED_TRACE(step);
   const auto action = random() % 5;
   if (used.empty() || (action == 0 && used.size() < 96))
   {
    used.resize(used.size() + 1 + random() % 4, false);
    fk.resize(Record_Id(index_t(used.size())));
   }
   else if (action == 1)
   {
    const Record_Id id = fk.get_free_record();
    used.resize(fk.size(), false);
    ASSERT_GE(to_underlying(id), 0);
    ASSERT_LT(size_t(id), used.size());
    ASSERT_FALSE(used[size_t(id)]);
    fk.use(id);
    used[size_t(id)] = true;
   }
   else
   {
    const size_t first = random() % used.size();
    size_t count = 1;
    while (count < 8 && first + count < used.size() &&
           used[first + count] == used[first])
     ++count;
    if (used[first])
     fk.free_vector(Record_Id(index_t(first)), count);
    else
     fk.use_vector(Record_Id(index_t(first)), count);
    const bool next = !used[first];
    for (size_t i = first; i < first + count; ++i)
     used[i] = next;
   }

   EXPECT_EQ(fk.size(), used.size());
   EXPECT_EQ(to_underlying(fk.get_used_count()), std::count(used.begin(), used.end(), true));
   for (size_t i = 0; i < used.size(); ++i)
    ASSERT_EQ(fk.is_used(Record_Id(index_t(i))), used[i]) << i;

   for (const bool want_used: {false, true})
   {
    const Record_Id sentinel = want_used ? fk.used_list : fk.free_list;
    Record_Id previous = sentinel;
    Record_Id current = fk.get_next(sentinel);
    std::vector<bool> visited(used.size(), false);
    size_t count = 0;
    while (current != sentinel)
    {
     ASSERT_LT(count++, used.size());
     ASSERT_GE(to_underlying(current), 0);
     ASSERT_LT(size_t(current), used.size());
     ASSERT_FALSE(visited[size_t(current)]);
     visited[size_t(current)] = true;
     ASSERT_EQ(used[size_t(current)], want_used);
     EXPECT_EQ(to_underlying(fk.get_previous(current)), to_underlying(previous));
     previous = current;
     current = fk.get_next(current);
    }
    EXPECT_EQ(fk.get_previous(sentinel), previous);
    EXPECT_EQ(count, size_t(std::count(used.begin(), used.end(), want_used)));
   }

   const size_t first = random() % (used.size() + 4);
   const size_t count = 1 + random() % 8;
   bool all_used = true;
   bool all_free = true;
   for (size_t i = first; i < first + count; ++i)
   {
    const bool is_used = i < used.size() && used[i];
    all_used = all_used && is_used;
    all_free = all_free && !is_used;
   }
   EXPECT_EQ(fk.is_used_vector(Record_Id(index_t(first)), count), all_used);
   EXPECT_EQ(fk.is_free_vector(Record_Id(index_t(first)), count), all_free);
  }
 }

}
