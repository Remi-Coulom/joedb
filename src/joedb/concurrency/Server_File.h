#ifndef joedb_Server_File_declared
#define joedb_Server_File_declared

#include "joedb/concurrency/Server_Connection.h"
#include "joedb/journal/Buffered_File.h"
#include "joedb/journal/Memory_File.h"

namespace joedb
{
 /// Directly read file served from joedb_server
 ///
 /// This class allows reading a remote file via the joedb network protocol.
 /// It is convenient for reading blobs from a large remote database without
 /// having to download a local replica. This file can also be written to:
 /// the head and tail of the file are stored in RAM locally, and can be
 /// pushed to the remote server if used in a client.
 ///
 /// \ingroup concurrency
 class Server_File: public Server_Connection, public Buffered_File
 {
  private:
   int64_t tail_offset;

   Memory_File head;
   Memory_File tail;

   size_t remote_pread(char *data, size_t size, int64_t offset) const;
   static void write_to_body_error();
   void write_checkpoint();

   int64_t pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait,
    char pull_type
   );

  public:
   Server_File(Channel &channel);

   //
   // Server_Connection overrides
   //
   int64_t handshake
   (
    Readonly_Journal &client_journal,
    bool content_check
   ) override;

   int64_t pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait
   ) override;

   int64_t lock_pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait
   ) override;

   int64_t push_until
   (
    Readonly_Journal &client_journal,
    int64_t server_position,
    int64_t until_position,
    bool unlock_after
   ) override;

   //
   // Abstract_File overrides
   //
   size_t pread(char *data, size_t size, int64_t offset) const override;
   void pwrite(const char *data, size_t size, int64_t offset) override;
   int64_t get_size() const override {return tail_offset + tail.get_size();}

   //
   // Buffered_File override
   //
   std::string read_blob_data(Blob blob) override;
 };
}

#endif
