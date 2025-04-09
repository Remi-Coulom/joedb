#ifndef joedb_File_Connection_declared
#define joedb_File_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/journal/File_Hasher.h"

namespace joedb
{
 /// @ingroup concurrency
 class Pullonly_Journal_Connection: public Connection
 {
  protected:
   Readonly_Journal &server_journal;

  public:
   //////////////////////////////////////////////////////////////////////////
   int64_t handshake
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &client_journal,
    bool content_check
   ) override
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
   int64_t pull
   //////////////////////////////////////////////////////////////////////////
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds
   ) override
   {
    server_journal.pull();
    client_journal.pull_from(server_journal);
    return server_journal.get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_checkpoint
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &client_journal,
    std::chrono::milliseconds
   ) override
   {
    server_journal.pull();
    return server_journal.get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   Pullonly_Journal_Connection(Readonly_Journal &server_journal):
   //////////////////////////////////////////////////////////////////////////
    server_journal(server_journal)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t lock_pull
   //////////////////////////////////////////////////////////////////////////
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds
   ) override
   {
    throw Exception("Connected to a read-only journal: can't lock");
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t push_until
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &client_journal,
    const int64_t from_checkpoint,
    const int64_t until_checkpoint,
    bool unlock_after
   ) override
   {
    throw Exception("Connected to a read-only journal: can't push");
   }
 };

 /// @ingroup concurrency
 class Journal_Connection: public Pullonly_Journal_Connection
 {
  private:
   Writable_Journal &get_journal()
   {
    return static_cast<Writable_Journal &>(server_journal);
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   int64_t lock_pull
   //////////////////////////////////////////////////////////////////////////
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds
   ) override
   {
    get_journal().lock_pull();
    client_journal.pull_from(server_journal);
    return server_journal.get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t push_until
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &client_journal,
    const int64_t from_checkpoint,
    const int64_t until_checkpoint,
    bool unlock_after
   ) override
   {
    if (!get_journal().is_locked())
     get_journal().lock_pull();

    static_cast<Writable_Journal &>(server_journal).pull_from
    (
     client_journal,
     until_checkpoint
    );

    if (unlock_after)
     get_journal().unlock();

    return server_journal.get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   void unlock() override
   //////////////////////////////////////////////////////////////////////////
   {
    get_journal().unlock();
   }

   //////////////////////////////////////////////////////////////////////////
   Journal_Connection(Writable_Journal &server_journal):
   //////////////////////////////////////////////////////////////////////////
    Pullonly_Journal_Connection(server_journal)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   ~Journal_Connection()
   //////////////////////////////////////////////////////////////////////////
   {
    get_journal().unlock();
   }
 };

 namespace detail
 {
  ///////////////////////////////////////////////////////////////////////////
  class File_Connection_Data
  ///////////////////////////////////////////////////////////////////////////
  {
   protected:
    Writable_Journal server_journal;

   public:
    /////////////////////////////////////////////////////////////////////////
    File_Connection_Data
    /////////////////////////////////////////////////////////////////////////
    (
     Buffered_File &server_file,
     Readonly_Journal::Check check,
     Commit_Level commit_level
    ):
    server_journal(server_file, check, commit_level)
    {
    }
  };
 }

 /// @ingroup concurrency
 class File_Connection:
  public detail::File_Connection_Data,
  public Journal_Connection
 {
  public:
   //////////////////////////////////////////////////////////////////////////
   File_Connection
   //////////////////////////////////////////////////////////////////////////
   (
    Buffered_File &server_file,
    Readonly_Journal::Check check = Readonly_Journal::Check::all,
    Commit_Level commit_level = Commit_Level::no_commit
   ):
   File_Connection_Data(server_file, check, commit_level),
   Journal_Connection(File_Connection_Data::server_journal)
   {
   }
 };
}

#endif
