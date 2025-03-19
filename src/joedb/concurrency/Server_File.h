#ifndef Server_File_declared
#define Server_File_declared

#include "joedb/concurrency/Server_Connection.h"
#include "joedb/journal/Generic_File.h"

namespace joedb
{
 class Server_File: public Server_Connection, public Generic_File
 {
  public:
   Server_File(Channel &channel);

   size_t pread(char *data, size_t size, int64_t offset) override;
   void pwrite(const char *data, size_t size, int64_t offset) override {};
   int64_t get_size() const override {return server_checkpoint;}
 };
}

#endif
