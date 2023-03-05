#ifndef joedb_Interpreter_declared
#define joedb_Interpreter_declared

#include "joedb/Type.h"
#include "joedb/Readable.h"
#include "joedb/Writable.h"

#include <iosfwd>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Interpreter
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   const Readable &readable;

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
   ) const;

   virtual bool process_command
   (
    const std::string &command,
    std::istream &iss,
    std::ostream &out
   );

  public:
   Readonly_Interpreter(const Readable &readable):
    readable(readable),
    echo(true),
    rethrow(false)
   {
   }

   void set_echo(bool new_echo) {echo = new_echo;}
   void set_rethrow(bool new_rethrow) {rethrow = new_rethrow;}

   void main_loop(std::istream &in, std::ostream &out);

   virtual ~Readonly_Interpreter() = default;
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreter: public Readonly_Interpreter
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable &writable;

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
   Interpreter
   (
    Readable &readable,
    Writable &writable,
    Record_Id max_record_id = 0
   ):
    Readonly_Interpreter(readable),
    writable(writable),
    max_record_id(max_record_id)
   {}

   void main_loop(std::istream &in, std::ostream &out);
 };
}

#endif
