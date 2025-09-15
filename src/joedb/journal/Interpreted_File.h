#ifndef joedb_Interpreted_File_declared
#define joedb_Interpreted_File_declared

#include "joedb/journal/File.h"
#include "joedb/journal/filebuf.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/interpreted/Database.h"
#include "joedb/Multiplexer.h"
#include "joedb/ui/Interpreter.h"

#include <iostream>

namespace joedb
{
 /// Store a file in joedbi format
 ///
 /// @ingroup journal
 class Interpreted_File: public Abstract_File
 {
  private:
   void read_data();

  protected:
   File file;
   joedb::filebuf filebuf;
   std::iostream ios;

   Memory_File memory_file;
   Writable_Journal journal;

   Database db;
   Multiplexer multiplexer;
   Interpreter interpreter;

   Abstract_File null_file;
   joedb::filebuf null_filebuf;
   std::ostream null_stream;

  public:
   Interpreted_File(const char *file_name, Open_Mode mode);

   void sync() override;
   void shared_lock(int64_t start, int64_t size) override;
   void exclusive_lock(int64_t start, int64_t size) override;
   void unlock(int64_t start, int64_t size) noexcept override;
   void pwrite(const char *buffer, size_t size, int64_t offset) override;
   size_t pread(char *data, size_t size, int64_t offset) const override;
 };
}

#endif
