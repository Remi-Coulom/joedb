#ifndef joedb_json_declared
#define joedb_json_declared

#include <iosfwd>
#include <string>

namespace joedb
{
 class Readable;

 /// @ingroup ui
 enum JSON_Error {ok = 0, utf8 = 1, infnan = 2};
 /// @ingroup ui
 int write_json(std::ostream &out, const Readable &db, bool base64);
 /// @ingroup ui
 int write_json_string(std::ostream &out, const std::string &s, bool base64);
}

#endif
