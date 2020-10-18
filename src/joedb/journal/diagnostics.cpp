#include "joedb/journal/diagnostics.h"
#include "joedb/journal/Generic_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/io/type_io.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void dump_header(std::ostream &out, Generic_File &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  file.set_position(0);

  out << "About this file\n";
  out << "---------------\n";

  {
   out << "joedb: ";
   std::string joedb;
   for (int i = 5; --i >= 0;)
    joedb.push_back(char(file.read<uint8_t>()));
   write_string(out, joedb);
   out << '\n';
  }

  {
   const uint32_t version = file.read<uint32_t>();
   out << "version: " << version << '\n';
  }

  {
   uint64_t pos[4];
   for (int i = 0; i < 4; i++)
    pos[i] = file.read<uint64_t>();

   for (int i = 0; i < 4; i++)
    out << "checkpoint[" << i << "] = " << pos[i] << '\n';
  }

  out << "file size: " << file.get_size() << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void about_joedb(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "About this version of joedb\n";
  out << "---------------------------\n";
  out << "Writable_Journal::compatible_version: ";
  out << Writable_Journal::compatible_version << '\n';
  out << "Writable_Journal::version_number: ";
  out << Writable_Journal::version_number << '\n';
  out << "version: " <<
  #include "../../VERSION"
  ;
  out << "\ncompiled: " << __DATE__ << ' ' << __TIME__ << '\n';
  out << "sizeof(size_t) = " << sizeof(size_t) << '\n';
  out << "sizeof(long) = " << sizeof(long) << '\n';
  out << "sizeof(std::streamoff) = " << sizeof(std::streamoff) << '\n';
#ifdef __unix__
  out << "sizeof(off_t) = " << sizeof(off_t) << '\n';
#endif
  out << "web site: https://www.remi-coulom.fr/joedb/\n";
 }
}
