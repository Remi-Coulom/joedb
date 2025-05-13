#include "tutorial/File_Client.h"

int main()
{
 tutorial::File_Client client("database.joedb");
 
 client.transaction([](tutorial::Writable_Database &db)
 {
  db.write_comment("Hello concurrency!");
 });

 return 0;
}
