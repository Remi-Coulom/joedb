#ifndef Server_File_declared
#define Server_File_declared

#include "joedb/concurrency/Server_Client.h"
#include "joedb/journal/Generic_File.h"

namespace joedb
{
 class Server_File: public Generic_File
 {
  private:
   Server_Client &client;

  public:
   Server_File(Server_Client &client);
   size_t pread(char *data, size_t size, int64_t offset) override;
   void pwrite(const char *data, size_t size, int64_t offset) override {};
   int64_t get_size() const override {return client.server_checkpoint;}
 };
}

#endif
