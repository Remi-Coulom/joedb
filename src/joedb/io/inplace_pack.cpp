#include "joedb/io/inplace_pack.h"
#include "joedb/io/dump.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Writable_Journal.h"

#include <cstdio>

namespace joedb
{
 void inplace_pack(const std::string &file_name)
 {
  const std::string packed = file_name + ".packed";

  {
   joedb::File input_file(file_name, joedb::Open_Mode::read_existing);
   joedb::File output_file(packed, joedb::Open_Mode::create_new);
   joedb::Readonly_Journal input_journal(input_file);
   joedb::Writable_Journal output_journal(output_file);
   joedb::pack(input_journal, output_journal);
  }

  const std::string unpacked = file_name + ".unpacked";

  if
  (
   std::rename(file_name.c_str(), unpacked.c_str()) ||
   std::rename(packed.c_str(), file_name.c_str())
  )
  {
   throw joedb::Runtime_Error("error moving files");
  }
 }
}
