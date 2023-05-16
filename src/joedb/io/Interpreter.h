#ifndef joedb_Interpreter_declared
#define joedb_Interpreter_declared

#include "joedb/Type.h"
#include "joedb/Readable.h"
#include "joedb/Writable.h"
#include "joedb/io/Command_Processor.h"

#include <iosfwd>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Interpreter: public Command_Processor
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   const Readable &readable;
   Blob_Reader * const blob_reader;

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

   Status process_command
   (
    const std::string &command,
    std::istream &iss,
    std::ostream &out
   ) override;

  public:
   Readonly_Interpreter
   (
    const Readable &readable,
    Blob_Reader *blob_reader
   ):
    readable(readable),
    blob_reader(blob_reader),
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
   Writable * const blob_writer;

   void update_value
   (
    std::istream &in,
    Table_Id table_id,
    Record_Id record_id,
    Field_Id field_id
   );

   Status process_command
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
    Blob_Reader *blob_reader,
    Writable *blob_writer,
    Record_Id max_record_id
   ):
    Readonly_Interpreter(readable, blob_reader),
    writable(writable),
    blob_writer(blob_writer),
    max_record_id(max_record_id)
   {}

   void main_loop(std::istream &in, std::ostream &out);
 };
}

#endif
