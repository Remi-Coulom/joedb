#include "joedb/compiler/Compiler_Options.h"
#include "joedb/compiler/Compiler_Options_io.h"

#include "gtest/gtest.h"

#include <sstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(Compiler_Options, unique_index)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::Compiler_Options::Index index;
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

 TEST(Compiler_Options, index_requires_fields)
 {
  for (const char *command: {"create_index", "create_unique_index",
                            "create_unordered_index", "create_unordered_unique_index"})
  {
   SCOPED_TRACE(command);
   joedb::Compiler_Options options;
   options.db.create_table("items");
   options.db.add_field(joedb::Table_Id(1), "value", joedb::Type::int32());
   std::istringstream input(std::string(command) + " lookup items\n");
   std::ostringstream errors;
   EXPECT_THROW(joedb::parse_compiler_options(input, options, errors), joedb::Exception);
   EXPECT_TRUE(options.get_indices().empty());
  }
 }
}
