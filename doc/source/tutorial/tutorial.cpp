#include "tutorial.h"

#include <iostream>

int main()
{
 tutorial::Database db("tutorial.joedb");

 if (!db.is_good())
 {
  std::cerr << "Error: could not open database\n";
  std::cerr << "State = " << (int)db.get_journal_state() << '\n';
  return 1;
 }
 else
  std::cout << "OK: db was opened successfully\n";

 return 0;
}
