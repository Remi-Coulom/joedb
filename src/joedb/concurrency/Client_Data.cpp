#include "joedb/concurrency/Client_Data.h"

namespace joedb
{
 Writable_Journal &Client_Data::get_writable_journal()
 {
  throw Exception("Client_Data has no writable journal");
 }

 Readonly_Journal &Client_Data::get_readonly_journal()
 {
  return get_writable_journal();
 }

 Client_Data::~Client_Data() = default;
}
