#include "db/testdb_test_base64.h"
#include "db/testdb_test_escape.h"
#include "db/testdb.h"

/////////////////////////////////////////////////////////////////////////////
static void test(const my_namespace::is_nested::testdb::Database &db)
/////////////////////////////////////////////////////////////////////////////
{
 for (auto person: db.get_person_table())
  std::cout << db.get_name(person) << '\n';
}

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 test(my_namespace::is_nested::testdb::get_embedded_test_base64());
 test(my_namespace::is_nested::testdb::get_embedded_test_escape());

 return 0;
}
