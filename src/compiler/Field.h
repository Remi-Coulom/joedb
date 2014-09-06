#ifndef crazydb_Field_declared
#define crazydb_Field_declared

namespace crazydb
{
 class Field
 {
  private:
   std::string name;
   Type type;

  public:
   const std::string &get_name() {return name;}
 };
}

#endif
