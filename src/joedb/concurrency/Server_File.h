#ifndef Server_File_declared
#define Server_File_declared

#include "joedb/concurrency/Server_Connection.h"
#include "joedb/journal/Generic_File.h"
#include "joedb/journal/Memory_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server_File: public Server_Connection, public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   const int64_t tail_offset;

   Memory_File head;
   Memory_File tail;

   size_t remote_pread(char *data, size_t size, int64_t offset);
   void write_to_body_error();

  public:
   Server_File(Channel &channel);

   size_t pread(char *data, size_t size, int64_t offset) override;
   void pwrite(const char *data, size_t size, int64_t offset) override;
   std::string read_blob_data(Blob blob) override;
   int64_t get_size() const override {return tail_offset + tail.get_size();}
 };
}

#endif
