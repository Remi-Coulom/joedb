#ifndef Sample_Declared
#define Sample_Declared

#include <fstream>

class Sample
{
 private:
  std::string file_prefix;
  std::ofstream ofsLog;

 public:
  Sample(const std::string &file_prefix);
 
  class City
  {
  };

  class Person
  {
  };
};

#endif
