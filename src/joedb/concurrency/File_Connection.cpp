#include "joedb/concurrency/File_Connection.h"
#include "joedb/journal/File_Hasher.h"

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 int64_t Pullonly_Journal_Connection::handshake
 //////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  Content_Check content_check
 )
 {
  if (content_check != Content_Check::none)
  {
   const int64_t min = std::min
   (
    server_journal.get_checkpoint(),
    client_journal.get_checkpoint()
   );

   if
   (
    Journal_Hasher::get_hash(client_journal, min) !=
    Journal_Hasher::get_hash(server_journal, min)
   )
   {
    content_mismatch();
   }
  }

  return server_journal.get_checkpoint();
 }

 //////////////////////////////////////////////////////////////////////////
 int64_t Pullonly_Journal_Connection::pull
 //////////////////////////////////////////////////////////////////////////
 (
  Lock_Action lock_action,
  Data_Transfer data_transfer,
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  if (bool(lock_action))
   throw Exception("Connected to a read-only journal: can't lock");

  server_journal.pull();

  if (bool(data_transfer))
   client_journal.pull_from(server_journal);

  return server_journal.get_checkpoint();
 }

 //////////////////////////////////////////////////////////////////////////
 int64_t Pullonly_Journal_Connection::push
 //////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  const int64_t from,
  const int64_t until,
  Unlock_Action unlock_action
 )
 {
  throw Exception("Connected to a read-only journal: can't push");
 }

 //////////////////////////////////////////////////////////////////////////
 int64_t Journal_Connection::pull
 //////////////////////////////////////////////////////////////////////////
 (
  Lock_Action lock_action,
  Data_Transfer data_transfer,
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  if (bool(lock_action))
   get_journal().lock_pull();
  else
   get_journal().pull();

  if (bool(data_transfer))
   client_journal.pull_from(server_journal);

  return server_journal.get_checkpoint();
 }

 //////////////////////////////////////////////////////////////////////////
 int64_t Journal_Connection::push
 //////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  const int64_t from,
  const int64_t until,
  Unlock_Action unlock_action
 )
 {
  if (!get_journal().is_locked())
   get_journal().lock_pull();

  if (from != server_journal.get_checkpoint())
   throw Exception("push error: conflict");
  get_journal().pull_from(client_journal, until);

  if (bool(unlock_action))
   get_journal().unlock();

  return server_journal.get_checkpoint();
 }

 //////////////////////////////////////////////////////////////////////////
 void Journal_Connection::unlock()
 //////////////////////////////////////////////////////////////////////////
 {
  get_journal().unlock();
 }

 //////////////////////////////////////////////////////////////////////////
 Journal_Connection::~Journal_Connection()
 //////////////////////////////////////////////////////////////////////////
 {
  get_journal().unlock();
 }
}
