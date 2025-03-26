#include "joedb/ui/open_mode_strings.h"

namespace joedb::ui
{
 const std::array<const char *, size_t(Open_Mode::modes)> open_mode_strings =
 {
  "read",
  "write",
  "new",
  "write_or_new",
  "shared",
  "lock"
 };
}
