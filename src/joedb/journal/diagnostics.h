#ifndef joedb_diagnostics_declared
#define joedb_diagnostics_declared

#include <iosfwd>

namespace joedb
{
 class Buffered_File;

 void dump_header(std::ostream &out, Buffered_File &file);
 void about_joedb(std::ostream &out);
}

#endif
