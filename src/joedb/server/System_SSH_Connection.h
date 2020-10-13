#ifndef joedb_System_SSH_Connection_declared
#define joedb_System_SSH_Connection_declared

#include "joedb/server/Connection.h"
#include "joedb/journal/File.h"

#include <memory>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class System_SSH_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   static const std::string local_file_name;
   const std::string host;
   const std::string remote_file_name;

   std::unique_ptr<File> server_file;
   std::unique_ptr<Journal_File> server_journal;

   void run(const std::string &command);

   void lock() override;
   void unlock() override;
   void pull(Journal_File &client_journal) override;
   void push(Readonly_Journal &client_journal) override;

  public:
   System_SSH_Connection
   (
    std::string host,
    std::string remote_file_name
   );
 };
}

#endif
