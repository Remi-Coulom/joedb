#include "joedb/compiler/Compiler_Options.h"

#include <gtest/gtest.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(Compiler, unique_index)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::compiler::Compiler_Options::Index index;
  index.table_id = Table_Id{1};
  index.unique = true;
  index.field_ids.push_back(Field_Id{2});
  index.field_ids.push_back(Field_Id{4});
  index.field_ids.push_back(Field_Id{1});

  EXPECT_TRUE(index.is_trigger(Table_Id{1}, Field_Id{4}));
  EXPECT_FALSE(index.is_trigger(Table_Id{1}, Field_Id{1}));
  EXPECT_FALSE(index.is_trigger(Table_Id{1}, Field_Id{2}));
  EXPECT_FALSE(index.is_trigger(Table_Id{1}, Field_Id{3}));
  EXPECT_FALSE(index.is_trigger(Table_Id{1}, Field_Id{5}));
  EXPECT_FALSE(index.is_trigger(Table_Id{2}, Field_Id{4}));

  index.unique = false;
  EXPECT_TRUE(index.is_trigger(Table_Id{1}, Field_Id{1}));
  EXPECT_TRUE(index.is_trigger(Table_Id{1}, Field_Id{2}));
  EXPECT_TRUE(index.is_trigger(Table_Id{1}, Field_Id{4}));
  EXPECT_FALSE(index.is_trigger(Table_Id{1}, Field_Id{3}));
  EXPECT_FALSE(index.is_trigger(Table_Id{2}, Field_Id{4}));
 }
}
