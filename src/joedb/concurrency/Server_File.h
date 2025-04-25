#ifndef joedb_Server_File_declared
#define joedb_Server_File_declared

#include "joedb/concurrency/Robust_Connection.h"
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
 /// A demo can be found in write_server_blob.cpp:
 /// @include write_server_blob.cpp
 ///
 /// @ingroup concurrency
 class Server_File: public Robust_Connection, public Buffered_File
 {
  private:
   int64_t tail_offset;

   Memory_File head;
   Memory_File tail;

   static void write_to_body_error();
   void write_checkpoint();

  public:
   Server_File(const Connector &connector, std::ostream *log = nullptr);

   //
   // Connection overrides
   //
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
    const Readonly_Journal &client_journal,
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
 };
}

#endif
