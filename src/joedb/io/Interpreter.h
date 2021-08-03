#ifndef joedb_Interpreter_declared
#define joedb_Interpreter_declared

#include "joedb/Type.h"
#include "joedb/Readable_Writable.h"

#include <iosfwd>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Interpreter
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Readable &db;

   Type parse_type(std::istream &in, std::ostream &out) const;
   Table_Id parse_table(std::istream &in, std::ostream &out) const;

   bool echo;
   bool rethrow;

   void after_command
   (
    std::ostream &out,
    int64_t line_number,
    const std::string &line,
    const Exception *exception
   );

   virtual bool process_command
   (
    const std::string &command,
    std::istream &iss,
    std::ostream &out
   );

  public:
   Readonly_Interpreter(Readable &db):
    db(db),
    echo(true),
    rethrow(false)
   {
   }

   void set_echo(bool echo) {this->echo = echo;}
   void set_rethrow(bool rethrow) {this->rethrow = rethrow;}

   void main_loop(std::istream &in, std::ostream &out);
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreter: public Readonly_Interpreter
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Readable_Writable &db;

   void update_value
   (
    std::istream &in,
    Table_Id table_id,
    Record_Id record_id,
    Field_Id field_id
   );

   bool process_command
   (
    const std::string &command,
    std::istream &iss,
    std::ostream &out
   ) override;

   Record_Id max_record_id;

  public:
   Interpreter(Readable_Writable &db, Record_Id max_record_id = 0):
    Readonly_Interpreter(db),
    db(db),
    max_record_id(max_record_id)
   {}
 };
}

#endif
