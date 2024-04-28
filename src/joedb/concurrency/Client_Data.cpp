#include "joedb/concurrency/Client_Data.h"

namespace joedb
{
 Writable_Journal &Client_Data::get_writable_journal()
 {
  throw Runtime_Error("Client_Data has no writable journal");
 }

 Readonly_Journal &Client_Data::get_readonly_journal()
 {
  return get_writable_journal();
 }

 void Client_Data::pull_data()
 {
  get_readonly_journal().pull();
  update();
 }

 Client_Data::~Client_Data() = default;
}
