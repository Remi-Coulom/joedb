#ifndef joedb_json_declared
#define joedb_json_declared

#include <iosfwd>
#include <string_view>

namespace joedb
{
 class Readable;

 /// @ingroup ui
 enum JSON_Error {ok = 0, utf8 = 1, infnan = 2};
 /// @ingroup ui
 int write_json(std::ostream &out, const Readable &db, bool base64);
 /// @ingroup ui
 int write_json_string(std::ostream &out, std::string_view s, bool base64);
}

#endif
