#include "joedb/ui/open_mode_strings.h"

namespace joedb
{
 const std::array<const char *, size_t(Open_Mode::mode_count)> open_mode_strings
 {
  "read",
  "write",
  "new",
  "write_or_new",
  "shared",
  "lock"
 };
}
