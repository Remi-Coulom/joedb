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

 Connection::~Connection() = default;
}
