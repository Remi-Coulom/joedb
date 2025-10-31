#ifndef joedb_Interpreted_File_declared
#define joedb_Interpreted_File_declared

#include "joedb/journal/File.h"
#include "joedb/journal/iostream.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/interpreted/Database.h"
#include "joedb/Multiplexer.h"
#include "joedb/ui/Interpreter.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"

#include <iostream>

namespace joedb
{
 ///
 /// This class allows accessing a file in joedbi text format. It works by
 /// keeping a binary version of the file in memory, and keeping it in sync
 /// with the text file. An empty new line is written at each checkpoint.
 /// Opening the text file fails if it does not end with an empty new line.
 ///
 /// @ingroup journal
 class Abstract_Interpreted_File: public Abstract_File
 {
  private:
   void read_data();

   joedb::filebuf filebuf;
   std::iostream ios;

   Memory_File memory_file;
   Writable_Journal journal;

   Database db;

   Multiplexer reading_multiplexer;
   Interpreter interpreter;

   Interpreter_Writable interpreter_writable;
   Multiplexer writing_multiplexer;

   null_iostream null_stream;

  public:
   Abstract_Interpreted_File(Abstract_File &file);

   int64_t get_size() const override;
   size_t pread(char *data, size_t size, int64_t offset) const override;
   void pwrite(const char *buffer, size_t size, int64_t offset) override;
   void sync() override;
   void shared_lock(int64_t start, int64_t size) override;
   void exclusive_lock(int64_t start, int64_t size) override;
   void unlock(int64_t start, int64_t size) noexcept override;
 };

 namespace detail
 {
  class Interpreted_File_Parent
  {
   protected:
    File file;
   public:
    Interpreted_File_Parent(const char *file_name, Open_Mode mode):
     file(file_name, mode)
    {
    }
  };
 }

 /// @ingroup journal
 class Interpreted_File:
  private detail::Interpreted_File_Parent,
  public Abstract_Interpreted_File
 {
  public:
   Interpreted_File(const char *file_name, Open_Mode mode):
    detail::Interpreted_File_Parent(file_name, mode),
    Abstract_Interpreted_File(file)
   {
   }
 };
}

#endif
