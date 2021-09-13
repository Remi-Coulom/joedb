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

   int64_t private_pull(Writable_Journal &client_journal)
   {
    client_journal.refresh_checkpoint();
    return client_journal.get_checkpoint_position();
   }

   int64_t pull(Writable_Journal &client_journal) override
   {
    int64_t result;
    run_while_locked([&](){result = private_pull(client_journal);});
    return result;
   }

   int64_t lock_pull(Writable_Journal &client_journal) override
   {
    lock();
    return private_pull(client_journal);
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

  public:
   Local_Connection(const char *file_name):
    file(file_name, Open_Mode::shared_write)
   {
   }

   Local_Connection(const std::string &file_name):
    Local_Connection(file_name.c_str())
   {
   }

   Generic_File &get_file() {return file;}
 };
}

#endif
