#ifndef joedb_Interpreter_declared
#define joedb_Interpreter_declared

#include <iosfwd>

#include "joedb/Type.h"

namespace joedb
{
 class Readable_Writeable;

 class Interpreter
 {
  private:
   Readable_Writeable &db;

   Type parse_type(std::istream &in, std::ostream &out);
   Table_Id parse_table(std::istream &in, std::ostream &out);
   void update_value(std::istream &in,
                     Table_Id table_id,
                     Record_Id record_id,
                     Field_Id field_id);

  public:
   Interpreter(Readable_Writeable &db): db(db) {}

   void main_loop(std::istream &in, std::ostream &out);
 };
}

#endif
