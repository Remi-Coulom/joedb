#include "joedb/concurrency/Connection.h"

namespace joedb
{
 void Connection::content_mismatch()
 {
  throw Exception("Client data does not match the server");
 }

 void Connection::check_shared(Readonly_Journal &client_journal)
 {
  if (!client_journal.is_shared())
   throw Exception("File must be shared");
 }

 void Connection::check_not_shared(Readonly_Journal &client_journal)
 {
  if (client_journal.is_shared())
   throw Exception("File cannot be shared");
 }

 void Connection::lock(Readonly_Journal &client_journal)
 {
  if (is_readonly())
   throw Exception("Readonly connection, lock not supported");
 }

 void Connection::push
 (
  Readonly_Journal &client_journal,
  int64_t server_checkpoint,
  bool unlock_after
 )
 {
  if (is_readonly())
   throw Exception("Readonly connection, push not supported");
 }

 Connection::~Connection() = default;
}
