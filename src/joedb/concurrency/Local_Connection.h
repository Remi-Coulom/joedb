#ifndef joedb_Local_Connection_declared
#define joedb_Local_Connection_declared

#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 template<typename File_Type> class Local_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   File_Type file;

   int64_t handshake() final override
   {
    int64_t result = 0;

    run_while_locked([&]()
    {
     Writable_Journal journal(file);
     result = journal.get_checkpoint_position();
    });

    return result;
   }

   void lock() final override
   {
    file.lock();
   }

   void unlock() final override
   {
    file.unlock();
   }

   int64_t pull(Writable_Journal &client_journal) final override
   {
    throw Exception("Pulling into a shared file without locking");
   }

   int64_t lock_pull(Writable_Journal &client_journal) final override
   {
    lock();
    client_journal.refresh_checkpoint();
    return client_journal.get_checkpoint_position();
   }

   int64_t lock_pull_unlock(Writable_Journal &client_journal) final override
   {
    run_while_locked([&]()
    {
     client_journal.refresh_checkpoint();
    });
    return client_journal.get_checkpoint_position();
   }

   void push
   (
    Readonly_Journal &client_journal,
    int64_t server_position,
    bool unlock_after
   ) final override
   {
    if (unlock_after)
     unlock();
   }

   bool check_matching_content
   (
    Readonly_Journal &client_journal,
    int64_t checkpoint
   ) final override
   {
    return client_journal.is_same_file(file);
   }

  public:
   Local_Connection(const char *file_name):
    file(file_name, Open_Mode::shared_write)
   {
   }

   Local_Connection(const std::string &file_name):
    Local_Connection(file_name.c_str())
   {
   }

   File_Type &get_file() {return file;}
 };
}

#endif
