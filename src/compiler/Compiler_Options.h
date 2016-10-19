#ifndef joedb_Compiler_Options_declared
#define joedb_Compiler_Options_declared

#include <string>

namespace joedb
{
 class Database;

 class Compiler_Options
 {
  private:
   const Database &db;

   std::string namespace_name;

  public:
   Compiler_Options(const Database &db): db(db) {}

   void set_namespace_name(const std::string &s) {namespace_name = s;}

   const Database &get_db() const {return db;}
   const std::string &get_namespace_name() const {return namespace_name;}
 };
}

#endif
