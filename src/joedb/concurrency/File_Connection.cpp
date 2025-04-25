#include "joedb/concurrency/File_Connection.h"
#include "joedb/journal/File_Hasher.h"

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 int64_t Pullonly_Journal_Connection::handshake
 //////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  bool content_check
 )
 {
  if (content_check)
  {
   const int64_t min = std::min
   (
    server_journal.get_checkpoint_position(),
    client_journal.get_checkpoint_position()
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

  return server_journal.get_checkpoint_position();
 }

 //////////////////////////////////////////////////////////////////////////
 int64_t Pullonly_Journal_Connection::pull
 //////////////////////////////////////////////////////////////////////////
 (
  bool lock_before,
  std::chrono::milliseconds wait,
  Writable_Journal *client_journal
 )
 {
  if (lock_before)
   throw Exception("Connected to a read-only journal: can't lock");

  server_journal.pull();

  if (client_journal)
   client_journal->pull_from(server_journal);

  return server_journal.get_checkpoint_position();
 }

 //////////////////////////////////////////////////////////////////////////
 int64_t Pullonly_Journal_Connection::push
 //////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  const int64_t from,
  const int64_t until,
  bool unlock_after
 )
 {
  throw Exception("Connected to a read-only journal: can't push");
 }

 //////////////////////////////////////////////////////////////////////////
 int64_t Journal_Connection::pull
 //////////////////////////////////////////////////////////////////////////
 (
  bool lock_before,
  std::chrono::milliseconds wait,
  Writable_Journal *client_journal
 )
 {
  if (lock_before)
   get_journal().lock_pull();
  else
   get_journal().pull();

  if (client_journal)
   client_journal->pull_from(server_journal);

  return server_journal.get_checkpoint_position();
 }

 //////////////////////////////////////////////////////////////////////////
 int64_t Journal_Connection::push
 //////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  const int64_t from,
  const int64_t until,
  bool unlock_after
 )
 {
  if (!get_journal().is_locked())
   get_journal().lock_pull();

  get_journal().pull_from(client_journal, until);

  if (unlock_after)
   get_journal().unlock();

  return server_journal.get_checkpoint_position();
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
