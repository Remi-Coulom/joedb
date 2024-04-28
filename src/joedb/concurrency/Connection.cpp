#include "joedb/concurrency/Connection.h"

namespace joedb
{
 void Connection::content_mismatch()
 {
  throw Exception("Client data does not match the server");
 }

 void Connection::check_not_shared(Readonly_Journal &client_journal)
 {
  if (client_journal.is_shared())
   throw Exception("File cannot be shared for this connection type");
 }

 void Connection::lock(Readonly_Journal &client_journal)
 {
 }

 void Connection::push
 (
  Readonly_Journal &client_journal,
  int64_t server_checkpoint,
  bool unlock_after
 )
 {
 }

 Connection::~Connection() = default;
}
