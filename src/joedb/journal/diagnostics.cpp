#include "joedb/journal/diagnostics.h"
#include "joedb/journal/Generic_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/File.h"
#include "joedb/io/type_io.h"
#include "joedb/get_version.h"

#ifdef JOEDB_HAS_SSH
#include <libssh/libssh.h>
#endif

#ifdef JOEDB_HAS_ASIO_NET
#include <asio/version.hpp>
#endif

#include <istream>
#include <ostream>

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
  out << "joedb version: " << get_version() << '\n';

#ifdef JOEDB_HAS_SSH
  out << "libssh version: " << ssh_version(0) << " https://www.libssh.org/\n";
#endif

#ifdef JOEDB_HAS_ASIO_NET
  out << "asio version: ";
  out << ASIO_VERSION / 100000 << '.';
  out << ASIO_VERSION / 100 % 1000 << '.';
  out << ASIO_VERSION % 100;
  out << " https://think-async.com/Asio/\n";
#endif

  out << "compiled: " << __DATE__ << ' ' << __TIME__ << '\n';
  out << "sizeof(size_t) = " << sizeof(size_t) << '\n';
  out << "sizeof(long) = " << sizeof(long) << '\n';
  out << "sizeof(std::streamoff) = " << sizeof(std::streamoff) << '\n';
#ifdef __unix__
  out << "sizeof(off_t) = " << sizeof(off_t) << '\n';
#endif
  out << "File = " << JOEDB_INCLUDE(JOEDB_FILE, h) << '\n';
  out << "web site: https://www.remi-coulom.fr/joedb/\n";
 }
}
