#include "tutorial.h"

/////////////////////////////////////////////////////////////////////////////
int main()
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
