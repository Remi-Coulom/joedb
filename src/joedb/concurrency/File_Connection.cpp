#include "joedb/concurrency/File_Connection.h"

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
  check_matching_content
  (
   client_journal,
   content_check,
   Header::ssize,
   std::min(server_journal.get_checkpoint(), client_journal.get_checkpoint())
  );

  return server_journal.get_checkpoint();
 }

 //////////////////////////////////////////////////////////////////////////
 void Pullonly_Journal_Connection::check_matching_content
 //////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  Content_Check content_check,
  int64_t from,
  int64_t until
 )
 {
  if (content_check == Content_Check::none || from >= until)
   return;

  const int64_t fast_size = 1 << 11;
  const int64_t size = until - from;

  if (content_check == Content_Check::full || size <= 2 * fast_size)
  {
   if (!client_journal.equal_to(server_journal, from, until))
    this->content_mismatch();
  }
  else
  {
   if
   (
    !client_journal.equal_to(server_journal, from, from + fast_size) ||
    !client_journal.equal_to(server_journal, until - fast_size, until)
   )
   {
    this->content_mismatch();
   }
  }
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
 Journal_Connection::Journal_Connection(Writable_Journal &server_journal):
 //////////////////////////////////////////////////////////////////////////
  Pullonly_Journal_Connection(server_journal)
 {
  JOEDB_RELEASE_ASSERT(!server_journal.is_shared());
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
  if (from != server_journal.get_checkpoint())
   throw Exception("push error: conflict");
  get_journal().pull_from(client_journal, until);
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
