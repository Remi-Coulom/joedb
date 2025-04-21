#ifndef joedb_File_Connection_declared
#define joedb_File_Connection_declared

#include "joedb/concurrency/Connection.h"

namespace joedb
{
 /// @ingroup concurrency
 class Pullonly_Journal_Connection: public Connection
 {
  protected:
   Readonly_Journal &server_journal;

  public:
   Pullonly_Journal_Connection(Readonly_Journal &server_journal):
    server_journal(server_journal)
   {
   }

   int64_t handshake
   (
    const Readonly_Journal &client_journal,
    bool content_check
   ) override;

   int64_t pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds
   ) override;

   int64_t get_checkpoint
   (
    const Readonly_Journal &client_journal,
    std::chrono::milliseconds
   ) override;

   int64_t lock_pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds
   ) override;

   int64_t push_until
   (
    const Readonly_Journal &client_journal,
    const int64_t from_checkpoint,
    const int64_t until_checkpoint,
    bool unlock_after
   ) override;
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
   Journal_Connection(Writable_Journal &server_journal):
    Pullonly_Journal_Connection(server_journal)
   {
   }

   int64_t lock_pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds
   ) override;

   int64_t push_until
   (
    const Readonly_Journal &client_journal,
    const int64_t from_checkpoint,
    const int64_t until_checkpoint,
    bool unlock_after
   ) override;

   void unlock() override;

   ~Journal_Connection();
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
     Readonly_Journal::Check check
    ):
    server_journal(server_file, check)
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
    Readonly_Journal::Check check = Readonly_Journal::Check::all
   ):
   File_Connection_Data(server_file, check),
   Journal_Connection(File_Connection_Data::server_journal)
   {
   }
 };
}

#endif
