#ifndef joedb_Open_Mode_declared
#define joedb_Open_Mode_declared

namespace joedb
{
 /// \ingroup journal
 enum class Open_Mode
 {
  read_existing,
  write_existing,
  create_new,
  write_existing_or_create_new,
  shared_write,
  write_lock,
  modes
 };
}

#endif
