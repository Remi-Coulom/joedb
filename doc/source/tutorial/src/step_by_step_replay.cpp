#include "tutorial/Database_Writable.h"
#include "joedb/journal/File.h"

#include <iostream>
#include <iomanip>

namespace example
{
 ////////////////////////////////////////////////////////////////////////////
 class My_Database: public tutorial::Database_Writable
 ////////////////////////////////////////////////////////////////////////////
 {
  void comment(const std::string &comment) override
  {
   std::cout << "Comment: " << comment << '\n';
  }

  void timestamp(int64_t timestamp) override
  {
   std::cout << "Timestamp: " << timestamp << '\n';
  }
 };
}

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 example::My_Database db;
 joedb::File file("tutorial.joedb", joedb::Open_Mode::read_existing);
 joedb::Readonly_Journal journal(file);

 int counter = 0;

 while (journal.get_position() < journal.get_checkpoint())
 {
  journal.one_step(db);
  ++counter;

  std::cout << std::setw(3) << counter << ' ';
  std::cout << "Number of persons: ";
  std::cout << db.get_person_table().get_size();
  std::cout << '\n';
 }

 return 0;
}
