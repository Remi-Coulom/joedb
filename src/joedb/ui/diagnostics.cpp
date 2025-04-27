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

  out << "    file size: " << file.get_size();

  for (int i = 0; i < 4; i++)
   out << "\ncheckpoint[" << i << "]: " << header.checkpoint[i];

  out << "\nformat version: " << header.version;
  out << "\nsignature: ";
  write_string
  (
   out,
   std::string(header.signature.data(), header.signature.size())
  );
  out << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void about_joedb(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "About this version of joedb\n"
         "---------------------------\n"
         "joedb version: " << get_version();
  out << " https://www.joedb.org/\nfile format version: ";
  out << Readonly_Journal::format_version;

#ifdef JOEDB_HAS_SSH
  out << "\nlibssh version: " << ssh_version(0) << " https://www.libssh.org/";
#endif

#ifdef JOEDB_HAS_ASIO
  out << "\nasio version: ";
  out << ASIO_VERSION / 100000 << '.';
  out << ASIO_VERSION / 100 % 1000 << '.';
  out << ASIO_VERSION % 100;
  out << " https://think-async.com/Asio/";
#endif

#ifdef JOEDB_HAS_CURL
  out << "\ncurl version: " << curl_version();
#endif

#ifdef JOEDB_HAS_BROTLI
  out << std::hex;
  out << "\nbrotli decoder version: " << BrotliDecoderVersion();
  out << "\nbrotli encoder version: " << BrotliEncoderVersion();
  out << std::dec;
#endif

  out << "\ncompiled: " << __DATE__ << ' ' << __TIME__;
  out << "\nsizeof(bool) = " << sizeof(bool);
  out << "\nsizeof(size_t) = " << sizeof(size_t);
  out << "\nsizeof(long) = " << sizeof(long);
  out << "\nsizeof(std::streamoff) = " << sizeof(std::streamoff);
#ifdef __unix__
  out << "\nsizeof(off_t) = " << sizeof(off_t);
#endif
  out << "\nFile = " << JOEDB_INCLUDE(JOEDB_FILE, h);
#ifdef JOEDB_HAS_BROKEN_POSIX_LOCKING
  out << "\nbroken_posix_locking = true";
#else
  out << "\nbroken_posix_locking = false";
#endif
#ifdef _POSIX_SYNCHRONIZED_IO
  out << "\n_POSIX_SYNCHRONIZED_IO = " << _POSIX_SYNCHRONIZED_IO;
#endif
  out << '\n';
 }
}
