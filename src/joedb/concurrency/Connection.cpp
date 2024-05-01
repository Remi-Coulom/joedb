#include "joedb/concurrency/Connection.h"

namespace joedb
{
 void Readonly_Connection::content_mismatch()
 {
  throw Exception("Client data does not match the server");
 }

 Readonly_Connection::~Readonly_Connection() = default;
}
