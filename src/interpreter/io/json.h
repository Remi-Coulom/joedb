#ifndef joedb_json_declared
#define joedb_json_declared

#include <iosfwd>

namespace joedb
{
 class Readable;

 bool write_json(std::ostream &out, const Readable &db, bool base64);
}

#endif
