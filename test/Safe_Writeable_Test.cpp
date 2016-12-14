#include "Safe_Writeable.h"

#include "gtest/gtest.h"

#include <fstream>
#include <sstream>

using namespace joedb;

// TODO: test that all successful operation are actually performed
// TODO: check exception name

/////////////////////////////////////////////////////////////////////////////
TEST(Safe_Writeable_Test, test_all_errors)
/////////////////////////////////////////////////////////////////////////////
{
 #define CHECK_EXCEPTION(x)\
 {\
  bool exception_caught = false;\
  try {x;}\
  catch (std::runtime_error e) {exception_caught = true;}\
  EXPECT_TRUE(exception_caught);\
 }

 joedb::Safe_Writeable writeable(1000);
 const Database &db = writeable.get_db();

 writeable.create_table("person");
 CHECK_EXCEPTION(writeable.create_table("person"));

 {
  const table_id_t table_id = db.find_table("person");
  writeable.drop_table(table_id);
  CHECK_EXCEPTION(writeable.drop_table(table_id));
  CHECK_EXCEPTION(writeable.rename_table(table_id, "toto"));
 }

 writeable.create_table("cityx");
 const table_id_t table_id = db.find_table("cityx");
 writeable.rename_table(table_id, "city");
 CHECK_EXCEPTION(writeable.rename_table(table_id, "city"));
 writeable.add_field(table_id, "N", joedb::Type::string());
 CHECK_EXCEPTION(writeable.add_field(table_id, "N", joedb::Type::string()));
 CHECK_EXCEPTION(writeable.add_field(1234, "", joedb::Type::string()));
 writeable.rename_field(table_id, 1, "toto");
 CHECK_EXCEPTION(writeable.rename_field(table_id, 1, "toto"));
 CHECK_EXCEPTION(writeable.rename_field(1234, 1, "toto"));
 CHECK_EXCEPTION(writeable.rename_field(table_id, 1234, "toto"));

 CHECK_EXCEPTION(writeable.drop_field(table_id, 1234));
 CHECK_EXCEPTION(writeable.drop_field(1234, 1));
 writeable.drop_field(table_id, 1);

 writeable.custom("custom");
 writeable.comment("comment");
 writeable.timestamp(0);
 writeable.valid_data();

 writeable.add_field(table_id, "field", joedb::Type::string());
 const field_id_t field_id = db.get_tables().find(table_id)->second.find_field("field");
 writeable.insert(table_id, 1);
 CHECK_EXCEPTION(writeable.insert(1234, 0));
 CHECK_EXCEPTION(writeable.insert(table_id, 0));
 CHECK_EXCEPTION(writeable.insert(table_id, 1000000));
 //CHECK_EXCEPTION(writeable.insert(table_id, 1));
 writeable.insert_vector(table_id, 2, 10);
 CHECK_EXCEPTION(writeable.insert_vector(1234, 0, 10))
 CHECK_EXCEPTION(writeable.insert_vector(table_id, 0, 10))
 CHECK_EXCEPTION(writeable.insert_vector(table_id, 10, 0))
 CHECK_EXCEPTION(writeable.insert_vector(table_id, 100000, 1))
 CHECK_EXCEPTION(writeable.insert_vector(table_id, 100, 100000))

 writeable.delete_record(table_id, 1);
 CHECK_EXCEPTION(writeable.delete_record(12345, 1));
 //CHECK_EXCEPTION(writeable.delete(table_id, 1));
 //CHECK_EXCEPTION(writeable.delete(table_id, 2));

 writeable.insert(table_id, 1);
 writeable.update_string(table_id, 1, field_id, "toto");
 CHECK_EXCEPTION(writeable.update_string(table_id, 23456, field_id, "toto"));
 CHECK_EXCEPTION(writeable.update_string(table_id, 1, 2345, "toto"));
 CHECK_EXCEPTION(writeable.update_string(12345, 1, field_id, "toto"));
 CHECK_EXCEPTION(writeable.update_int32(table_id, 1, field_id, 12345));
}
