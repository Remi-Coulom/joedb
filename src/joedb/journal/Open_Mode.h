#ifndef joedb_Open_Mode_declared
#define joedb_Open_Mode_declared

namespace joedb
{
 /// @ingroup journal
 enum class Open_Mode
 {
  read_existing,                ///< fails if does not exist
  write_existing,               ///< fails if does not exist or locked, locks the file for writing
  create_new,                   ///< fails if already exists, locks the file for writing
  write_existing_or_create_new, ///< either write_existing or create_new depending on whether the file exists. Racy in Posix, not in Windows.
  shared_write,                 ///< like write_existing_or_create_new, but does not lock the file, and does not fail if locked
  write_lock,                   ///< like write_existing_or_create_new, but waits instead of failing if already locked
  truncate,                     ///< create new file, or truncate existing file, and locks the file
  mode_count                    ///< number of modes
 };
}

#endif
