#include "testdb.h"
#include "File.h"
#include "Journal_File.h"

#include <iostream>

int main()
{
 testdb::database db("test.joedb");

 if (!db.is_good())
 {
  std::cerr << "Error opening database\n";
  return 1;
 }

 std::cout << "Database opened successfully\n";

 return 0;
}
