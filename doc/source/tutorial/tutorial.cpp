#include "tutorial.h"

#include <iostream>

int main()
{
 tutorial::Database db("tutorial.joedb");

 if (!db.is_good())
 {
  std::cerr << "Error: could not open database\n";
  return 1;
 }

 return 0;
}
