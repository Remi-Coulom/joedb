#ifndef joedb_diagnostics_declared
#define joedb_diagnostics_declared

#include <iosfwd>

namespace joedb
{
 class Abstract_File;

 /// @ingroup journal
 void dump_header(std::ostream &out, Abstract_File &file);
 /// @ingroup journal
 void about_joedb(std::ostream &out);
}

#endif
