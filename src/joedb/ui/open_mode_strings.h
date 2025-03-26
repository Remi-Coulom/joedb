#ifndef joedb_open_mode_strings_declared
#define joedb_open_mode_strings_declared

#include "joedb/journal/Open_Mode.h"

#include <stddef.h>
#include <array>

namespace joedb::ui
{
 extern const std::array<const char *, size_t(Open_Mode::modes)> open_mode_strings;
}

#endif
