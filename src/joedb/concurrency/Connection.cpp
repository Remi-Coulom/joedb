#include "joedb/concurrency/Connection.h"

namespace joedb
{
 Content_Mismatch::Content_Mismatch(): Exception
 (
  "Content mismatch: the file and the connection have diverged"
 )
 {
 }

 void Connection::content_mismatch()
 {
  throw Content_Mismatch();
 }

 int64_t Connection::handshake
 (
  Readonly_Journal &client_journal,
  bool content_check
 )
 {
  return client_journal.get_checkpoint();
 }

 int64_t Connection::pull
 (
  bool lock_before,
  bool write_data,
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  return client_journal.get_checkpoint();
 }

 int64_t Connection::push
 (
  Readonly_Journal &client_journal,
  int64_t from,
  int64_t until,
  bool unlock_after
 )
 {
  return client_journal.get_checkpoint();
 }

 void Connection::unlock()
 {
 }

 Connection::~Connection() = default;
}
