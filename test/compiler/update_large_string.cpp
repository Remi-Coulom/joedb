#include <iostream>

#include "db/schema_v1.h"

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 {
  schema_v1::Generic_File_Database db(file);

  auto person = db.new_person();

  db.set_name(person, std::string(1000000000, 'X'));
  db.checkpoint_no_commit();
 }

 {
  schema_v1::Readonly_Database db(file);
 }

 return 0;
}
