#include "testdb.h"
#include "File.h"
#include "Journal_File.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 std::cout << "\nTesting compiled code...\n";

 //
 // First, try to open the database
 //
 testdb::Database db("test.joedb");
 if (!db.is_good())
 {
  std::cerr << "Error opening database\n";
  return 1;
 }
 else
  std::cout << "Database opened successfully\n";

 return 0;
}
