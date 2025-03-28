#ifndef joedb_diagnostics_declared
#define joedb_diagnostics_declared

#include <iosfwd>

namespace joedb
{
 class Buffered_File;

 /// \ingroup journal
 void dump_header(std::ostream &out, Buffered_File &file);
 /// \ingroup journal
 void about_joedb(std::ostream &out);
}

#endif
