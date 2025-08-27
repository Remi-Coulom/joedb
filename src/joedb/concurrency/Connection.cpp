#include "joedb/concurrency/Connection.h"

namespace joedb
{
 Content_Mismatch::Content_Mismatch(): Disconnection
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
  const Readonly_Journal &client_journal,
  Content_Check content_check
 )
 {
  return client_journal.get_checkpoint();
 }

 int64_t Connection::pull
 (
  Lock_Action lock_action,
  Data_Transfer data_transfer,
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  return client_journal.get_checkpoint();
 }

 int64_t Connection::push
 (
  const Readonly_Journal &client_journal,
  int64_t from,
  int64_t until,
  Unlock_Action unlock_action
 )
 {
  return client_journal.get_checkpoint();
 }

 void Connection::unlock()
 {
 }

 bool Connection::is_pullonly() const
 {
  return false;
 }

 Connection Connection::dummy;

 Connection::~Connection() = default;
}
