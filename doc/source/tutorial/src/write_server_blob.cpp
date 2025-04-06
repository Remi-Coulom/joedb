#include "joedb/ui/main_exception_catcher.h"
#include "joedb/concurrency/Network_Channel.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/error/Posthumous_Catcher.h"

#include <iostream>

/// Demonstration of joedb::Server_File
///
/// This demonstrates how to connect to a joedb server to read and write blobs,
/// without downloading a full replica of the database.
static int write_server_blob(int argc, char **argv)
{
 if (argc < 3)
 {
  std::cerr << "usage: " << argv[0] << " <port> <blob_string>\n";
  std::cerr << "This program will try to connect to a server on localhost.\n";
  std::cerr << "Before running this program, start a joedb server with:\n";
  std::cerr << "joedb_server -port <port> blobs.joedb\n";
  std::cerr << "You can interactively read and write blobs this way:\n";
  std::cerr << "joedb_client --nodb server network localhost <port>\n";
  return 1;
 }

 // Connect to the server
 joedb::Network_Channel channel("localhost", argv[1]);
 joedb::Server_File server_file(channel);

 // Creating the client: server file serves both as file and connection
 joedb::Writable_Journal_Client client(server_file, server_file);

 // Write blobs with a Client_Lock: keeps the server locked between writes
 {
  joedb::Posthumous_Catcher catcher;

  {
   joedb::Writable_Journal_Client_Lock lock(client);
   lock.set_catcher(catcher);

   for (int i = 3; --i >= 0;)
   {
    const joedb::Blob blob = lock.get_journal().write_blob_data(argv[2]);
    lock.push();
    std::cout << "wrote blob with lock: " << blob.get_position() << '\n';
    std::cout << "blob: " << server_file.read_blob_data(blob) << '\n';
   }
  }

  catcher.rethrow();
 }

 // Write blobs with a transaction: lock and unlock for each write
 for (int i = 3; --i >= 0;)
 {
  joedb::Blob blob;
  client.transaction([&blob, argv](joedb::Writable_Journal &journal)
  {
   blob = journal.write_blob_data(argv[2]);
  });
  std::cout << "wrote blob with transaction: " << blob.get_position() << '\n';
  std::cout << "blob: " << server_file.read_blob_data(blob) << '\n';
 }

 return 0;
}

int main(int argc, char **argv)
{
 return joedb::main_exception_catcher(write_server_blob, argc, argv);
}
