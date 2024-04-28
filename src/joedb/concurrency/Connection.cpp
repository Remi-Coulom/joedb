#include "joedb/concurrency/Connection.h"

namespace joedb
{
 void Connection::content_mismatch()
 {
  throw Exception("Client data does not match the server");
 }

 Connection::~Connection() = default;
}
