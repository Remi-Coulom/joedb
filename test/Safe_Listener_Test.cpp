#include "Safe_Listener.h"

#include "gtest/gtest.h"

#include <fstream>
#include <sstream>

using namespace joedb;

// TODO: test that all successful operation are actually performed
// TODO: check exception name

/////////////////////////////////////////////////////////////////////////////
TEST(Safe_Listener_Test, test_all_errors)
/////////////////////////////////////////////////////////////////////////////
{
 #define CHECK_EXCEPTION(x)\
 {\
  bool exception_caught = false;\
  try {x;}\
  catch (std::runtime_error e) {exception_caught = true;}\
  EXPECT_TRUE(exception_caught);\
 }

 joedb::Safe_Listener listener(1000);
 const Database &db = listener.get_db();

 listener.after_create_table("person");
 CHECK_EXCEPTION(listener.after_create_table("person"));

 {
  const table_id_t table_id = db.find_table("person");
  listener.after_drop_table(table_id);
  CHECK_EXCEPTION(listener.after_drop_table(table_id));
  CHECK_EXCEPTION(listener.after_rename_table(table_id, "toto"));
 }

 listener.after_create_table("cityx");
 const table_id_t table_id = db.find_table("cityx");
 listener.after_rename_table(table_id, "city");
 CHECK_EXCEPTION(listener.after_rename_table(table_id, "city"));
 listener.after_add_field(table_id, "N", joedb::Type::string());
 CHECK_EXCEPTION(listener.after_add_field(table_id, "N", joedb::Type::string()));
 CHECK_EXCEPTION(listener.after_add_field(1234, "", joedb::Type::string()));
 listener.after_rename_field(table_id, 1, "toto");
 CHECK_EXCEPTION(listener.after_rename_field(table_id, 1, "toto"));
 CHECK_EXCEPTION(listener.after_rename_field(1234, 1, "toto"));
 CHECK_EXCEPTION(listener.after_rename_field(table_id, 1234, "toto"));

 CHECK_EXCEPTION(listener.after_drop_field(table_id, 1234));
 CHECK_EXCEPTION(listener.after_drop_field(1234, 1));
 listener.after_drop_field(table_id, 1);

 listener.after_custom("custom");
 listener.after_comment("comment");
 listener.after_timestamp(0);
 listener.after_valid_data();

 listener.after_add_field(table_id, "field", joedb::Type::string());
 const field_id_t field_id = db.get_tables().find(table_id)->second.find_field("field");
 listener.after_insert(table_id, 1);
 CHECK_EXCEPTION(listener.after_insert(1234, 0));
 CHECK_EXCEPTION(listener.after_insert(table_id, 0));
 CHECK_EXCEPTION(listener.after_insert(table_id, 1000000));
 //CHECK_EXCEPTION(listener.after_insert(table_id, 1));
 listener.after_insert_vector(table_id, 2, 10);
 CHECK_EXCEPTION(listener.after_insert_vector(1234, 0, 10))
 CHECK_EXCEPTION(listener.after_insert_vector(table_id, 0, 10))
 CHECK_EXCEPTION(listener.after_insert_vector(table_id, 10, 0))
 CHECK_EXCEPTION(listener.after_insert_vector(table_id, 100000, 1))
 CHECK_EXCEPTION(listener.after_insert_vector(table_id, 100, 100000))

 listener.after_delete(table_id, 1);
 CHECK_EXCEPTION(listener.after_delete(12345, 1));
 //CHECK_EXCEPTION(listener.after_delete(table_id, 1));
 //CHECK_EXCEPTION(listener.after_delete(table_id, 2));

 listener.after_insert(table_id, 1);
 listener.after_update_string(table_id, 1, field_id, "toto");
 CHECK_EXCEPTION(listener.after_update_string(table_id, 23456, field_id, "toto"));
 CHECK_EXCEPTION(listener.after_update_string(table_id, 1, 2345, "toto"));
 CHECK_EXCEPTION(listener.after_update_string(12345, 1, field_id, "toto"));
 CHECK_EXCEPTION(listener.after_update_int32(table_id, 1, field_id, 12345));
}
