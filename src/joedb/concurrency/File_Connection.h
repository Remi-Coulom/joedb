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
    bool lock_before,
    std::chrono::milliseconds wait,
    Writable_Journal *client_journal
   ) override;

   int64_t push
   (
    const Readonly_Journal *client_journal,
    const int64_t from,
    const int64_t until,
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

   int64_t pull
   (
    bool lock_before,
    std::chrono::milliseconds wait,
    Writable_Journal *client_journal
   ) override;

   int64_t push
   (
    const Readonly_Journal *client_journal,
    const int64_t from,
    const int64_t until,
    bool unlock_after
   ) override;

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
    File_Connection_Data(Buffered_File &server_file):
    /////////////////////////////////////////////////////////////////////////
     server_journal(server_file)
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
   File_Connection(Buffered_File &server_file):
   //////////////////////////////////////////////////////////////////////////
    File_Connection_Data(server_file),
    Journal_Connection(File_Connection_Data::server_journal)
   {
   }
 };
}

#endif
