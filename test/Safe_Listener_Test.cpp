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

 listener.create_table("person");
 CHECK_EXCEPTION(listener.create_table("person"));

 {
  const table_id_t table_id = db.find_table("person");
  listener.drop_table(table_id);
  CHECK_EXCEPTION(listener.drop_table(table_id));
  CHECK_EXCEPTION(listener.rename_table(table_id, "toto"));
 }

 listener.create_table("cityx");
 const table_id_t table_id = db.find_table("cityx");
 listener.rename_table(table_id, "city");
 CHECK_EXCEPTION(listener.rename_table(table_id, "city"));
 listener.add_field(table_id, "N", joedb::Type::string());
 CHECK_EXCEPTION(listener.add_field(table_id, "N", joedb::Type::string()));
 CHECK_EXCEPTION(listener.add_field(1234, "", joedb::Type::string()));
 listener.rename_field(table_id, 1, "toto");
 CHECK_EXCEPTION(listener.rename_field(table_id, 1, "toto"));
 CHECK_EXCEPTION(listener.rename_field(1234, 1, "toto"));
 CHECK_EXCEPTION(listener.rename_field(table_id, 1234, "toto"));

 CHECK_EXCEPTION(listener.drop_field(table_id, 1234));
 CHECK_EXCEPTION(listener.drop_field(1234, 1));
 listener.drop_field(table_id, 1);

 listener.custom("custom");
 listener.comment("comment");
 listener.timestamp(0);
 listener.valid_data();

 listener.add_field(table_id, "field", joedb::Type::string());
 const field_id_t field_id = db.get_tables().find(table_id)->second.find_field("field");
 listener.insert(table_id, 1);
 CHECK_EXCEPTION(listener.insert(1234, 0));
 CHECK_EXCEPTION(listener.insert(table_id, 0));
 CHECK_EXCEPTION(listener.insert(table_id, 1000000));
 //CHECK_EXCEPTION(listener.insert(table_id, 1));
 listener.insert_vector(table_id, 2, 10);
 CHECK_EXCEPTION(listener.insert_vector(1234, 0, 10))
 CHECK_EXCEPTION(listener.insert_vector(table_id, 0, 10))
 CHECK_EXCEPTION(listener.insert_vector(table_id, 10, 0))
 CHECK_EXCEPTION(listener.insert_vector(table_id, 100000, 1))
 CHECK_EXCEPTION(listener.insert_vector(table_id, 100, 100000))

 listener.delete_record(table_id, 1);
 CHECK_EXCEPTION(listener.delete_record(12345, 1));
 //CHECK_EXCEPTION(listener.delete(table_id, 1));
 //CHECK_EXCEPTION(listener.delete(table_id, 2));

 listener.insert(table_id, 1);
 listener.update_string(table_id, 1, field_id, "toto");
 CHECK_EXCEPTION(listener.update_string(table_id, 23456, field_id, "toto"));
 CHECK_EXCEPTION(listener.update_string(table_id, 1, 2345, "toto"));
 CHECK_EXCEPTION(listener.update_string(12345, 1, field_id, "toto"));
 CHECK_EXCEPTION(listener.update_int32(table_id, 1, field_id, 12345));
}
