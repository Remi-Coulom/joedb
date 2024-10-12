#include "tutorial/writable.h"

#include <iostream>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////
class My_Database: public tutorial::Database
/////////////////////////////////////////////////////////////////////////////
{
 void comment(const std::string &comment) override
 {
  std::cout << "Comment: " << comment << '\n';
 };

 void timestamp(int64_t timestamp) override
 {
  std::cout << "Timestamp: " << timestamp << '\n';
 };
};

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 My_Database db;
 joedb::File file("tutorial.joedb", joedb::Open_Mode::read_existing);
 joedb::Readonly_Journal journal(file);

 int counter = 0;

 while (!journal.at_end_of_file())
 {
  journal.one_step(db);
  ++counter;

  std::cout << std::setw(3) << counter << ' ';
  std::cout << "Number of persons: ";
  std::cout << db.get_person_table().get_size();
  std::cout << '\n';;
 }

 return 0;
}
