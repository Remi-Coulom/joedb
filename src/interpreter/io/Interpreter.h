#ifndef joedb_Interpreter_declared
#define joedb_Interpreter_declared

#include <iosfwd>

#include "joedb/Type.h"

namespace joedb
{
 class Database;

 class Interpreter
 {
  private:
   Database &db;

   Type parse_type(std::istream &in, std::ostream &out);
   table_id_t parse_table(std::istream &in, std::ostream &out);
   bool update_value(std::istream &in,
                     table_id_t table_id,
                     record_id_t record_id,
                     field_id_t field_id);
   static bool too_big(record_id_t record_id);

  public:
   Interpreter(Database &db): db(db) {}

   void main_loop(std::istream &in, std::ostream &out);
 };
}

#endif
