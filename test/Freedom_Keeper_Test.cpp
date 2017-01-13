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
