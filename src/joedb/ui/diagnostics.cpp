#include "joedb/ui/diagnostics.h"
#include "joedb/journal/Buffered_File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/File.h"
#include "joedb/ui/type_io.h"
#include "joedb/get_version.h"

#ifdef JOEDB_HAS_SSH
#include <libssh/libssh.h>
#endif

#ifdef JOEDB_HAS_ASIO
#include <asio/version.hpp>
#endif

#ifdef JOEDB_HAS_CURL
#include <curl/curl.h>
#endif

#ifdef JOEDB_HAS_BROTLI
#include <brotli/decode.h>
#include <brotli/encode.h>
#endif

#include <ostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void dump_header(std::ostream &out, Buffered_File &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  Header header;
  file.pread((char *)(&header), Header::size, 0);

  out << "About this file\n";
  out << "---------------\n";

  for (int i = 0; i < 4; i++)
   out << "checkpoint[" << i << "] = " << header.checkpoint[i] << '\n';

  out << "version: " << header.version << '\n';

  out << "joedb: ";
  write_string
  (
   out,
   std::string(header.signature.data(), header.signature.size())
  );
  out << '\n';

  out << "file size: " << file.get_size() << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void about_joedb(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "About this version of joedb\n";
  out << "---------------------------\n";
  out << "joedb version: " << get_version() << '\n';
  out << "file format version: " << Readonly_Journal::format_version;

#ifdef JOEDB_HAS_SSH
  out << "libssh version: " << ssh_version(0) << " https://www.libssh.org/\n";
#endif

#ifdef JOEDB_HAS_ASIO
  out << "asio version: ";
  out << ASIO_VERSION / 100000 << '.';
  out << ASIO_VERSION / 100 % 1000 << '.';
  out << ASIO_VERSION % 100;
  out << " https://think-async.com/Asio/\n";
#endif

#ifdef JOEDB_HAS_CURL
  out << "curl version: " << curl_version() << '\n';
#endif

#ifdef JOEDB_HAS_BROTLI
  out << std::hex;
  out << "brotli decoder version: " << BrotliDecoderVersion() << '\n';
  out << "brotli encoder version: " << BrotliEncoderVersion() << '\n';
  out << std::dec;
#endif

  out << "compiled: " << __DATE__ << ' ' << __TIME__ << '\n';
  out << "sizeof(size_t) = " << sizeof(size_t) << '\n';
  out << "sizeof(long) = " << sizeof(long) << '\n';
  out << "sizeof(std::streamoff) = " << sizeof(std::streamoff) << '\n';
#ifdef __unix__
  out << "sizeof(off_t) = " << sizeof(off_t) << '\n';
#endif
  out << "File = " << JOEDB_INCLUDE(JOEDB_FILE, h) << '\n';
  out << "broken_posix_locking = ";
#ifdef JOEDB_HAS_BROKEN_POSIX_LOCKING
  out << 1;
#else
  out << 0;
#endif
  out << '\n';
  out << "web site: https://www.joedb.org/\n";
 }
}
