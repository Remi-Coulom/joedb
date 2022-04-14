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

   int64_t handshake(Readonly_Journal &client_journal) override
   {
    if (!client_journal.is_same_file(file))
     throw joedb::Exception("Local_Connection to wrong file");

    run_while_locked([&](){client_journal.refresh_checkpoint();});
    return client_journal.get_checkpoint_position();
   }

   void lock() final override
   {
    file.lock();
   }

   void unlock() final override
   {
    file.unlock();
   }

   int64_t pull(Writable_Journal &client_journal) override
   {
    run_while_locked([&](){client_journal.refresh_checkpoint();});
    return client_journal.get_checkpoint_position();
   }

   int64_t lock_pull(Writable_Journal &client_journal) override
   {
    lock();
    client_journal.refresh_checkpoint();
    return client_journal.get_checkpoint_position();
   }

   void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   ) override
   {
    unlock();
   }

   bool check_matching_content
   (
    Readonly_Journal &client_journal,
    int64_t checkpoint
   ) override
   {
    return true;
   }

   class Lock_Guard
   {
    private:
     File_Type &file;

    public:
     Lock_Guard(File_Type &file): file(file)
     {
      file.lock();
     }

     ~Lock_Guard()
     {
      file.unlock();
     }
   };

  public:
   Local_Connection(const char *file_name):
    file(file_name, Open_Mode::shared_write)
   {
    if (file.get_mode() != Open_Mode::create_new)
    {
     Lock_Guard lock(file);
     Readonly_Journal journal(file);
     const bool ignore_errors = false;
     const bool ignore_trailing = false;
     journal.check_size(ignore_errors, ignore_trailing);
    }
   }

   Local_Connection(const std::string &file_name):
    Local_Connection(file_name.c_str())
   {
   }

   File_Type &get_file() {return file;}
 };
}

#endif
