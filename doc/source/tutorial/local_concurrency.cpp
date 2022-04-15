#include "tutorial.h"
#include "joedb/io/main_exception_catcher.h"

/////////////////////////////////////////////////////////////////////////////
static int local_concurrency(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Local_Connection<joedb::File> connection("local_concurrency.joedb");
 tutorial::Client client(connection);

 client.transaction([](tutorial::Generic_File_Database &db)
 {
  db.new_city("Tokyo");
  db.new_city("New York");
  db.new_city("Paris");
 });

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(local_concurrency, argc, argv);
}
