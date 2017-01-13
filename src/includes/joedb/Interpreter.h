#ifndef joedb_Interpreter_declared
#define joedb_Interpreter_declared

#include <iosfwd>

#include "Type.h"
#include "Readable_Writeable.h"

namespace joedb
{
/////////////////////////////////////////////////////////////////////////////
 class Readonly_Interpreter
/////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Readable &db;

   Type parse_type(std::istream &in, std::ostream &out);
   Table_Id parse_table(std::istream &in, std::ostream &out);

   virtual bool process_command
   (
    const std::string &line,
    std::istream &iss,
    const std::string &command,
    std::ostream &out
   );

  public:
   Readonly_Interpreter(Readable &db): db(db) {}

   void main_loop(std::istream &in, std::ostream &out);
 };

/////////////////////////////////////////////////////////////////////////////
 class Interpreter: public Readonly_Interpreter
/////////////////////////////////////////////////////////////////////////////
 {
  private:
   Readable_Writeable &db;

   void update_value(std::istream &in,
                     Table_Id table_id,
                     Record_Id record_id,
                     Field_Id field_id);

   bool process_command
   (
    const std::string &line,
    std::istream &iss,
    const std::string &command,
    std::ostream &out
   ) override;

  public:
   Interpreter(Readable_Writeable &db):
    Readonly_Interpreter(db),
    db(db)
   {}
 };
}

#endif
