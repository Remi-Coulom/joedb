#ifndef joedb_Interpreter_declared
#define joedb_Interpreter_declared

#include <iosfwd>

#include "Type.h"
#include "Value.h"

namespace joedb
{
 class Database;

 class Interpreter
 {
  private:
   Database &db;

   Type parse_type(std::istream &in, std::ostream &out);
   table_id_t parse_table(std::istream &in, std::ostream &out);
   static Value parse_value(Type::type_id_t type_id, std::istream &in);

  public:
   Interpreter(Database &db): db(db) {}

   void main_loop(std::istream &in, std::ostream &out);
 };
}

#endif
