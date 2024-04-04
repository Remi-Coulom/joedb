#include "joedb/io/open_mode_strings.h"

namespace joedb
{
 const std::array<const char *, size_t(Open_Mode::modes)> open_mode_strings =
 {
  "read_existing",
  "write_existing",
  "create_new",
  "write_existing_or_create_new",
  "shared_write",
  "write_lock"
 };
}
