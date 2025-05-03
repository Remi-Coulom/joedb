#include "joedb/ui/main_exception_catcher.h"
#include "joedb/concurrency/Local_Connector.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/concurrency/Writable_Journal_Client.h"

#include <iostream>

/// Demonstration of joedb::Server_File
///
/// This demonstrates how to connect to a joedb server to read and write blobs,
/// without downloading a full replica of the database.
static int write_server_blob(int argc, char **argv)
{
 if (argc < 2)
 {
  std::cerr << "usage: " << argv[0] << " <blob_string>\n";
  std::cerr << "This program will try to connect to a local server.\n";
  std::cerr << "Before running this program, start a joedb server with:\n";
  std::cerr << "joedb_server --socket blobs.sock blobs.joedb\n";
  std::cerr << "You can interactively read and write blobs this way:\n";
  std::cerr << "joedb_client --db none server local blobs.sock\n";
  return 1;
 }

 // Connect to the server
 joedb::Local_Connector connector("blobs.sock");
 joedb::Server_File server_file(connector, &std::cerr);

 // Creating the client: server file serves both as file and connection
 joedb::Writable_Journal_Client client(server_file, server_file);

 // Write blobs with a Client_Lock: keeps the server locked between writes
 {
  joedb::Writable_Journal_Client_Lock lock(client);

  for (int i = 3; --i >= 0;)
  {
   const joedb::Blob blob = lock.get_journal().write_blob(argv[1]);
   lock.push();
   std::cout << "wrote blob with lock: " << blob.get_position() << '\n';
   std::cout << "blob: " << server_file.read_blob(blob) << '\n';
  }

  lock.unlock();
 }

 // Write blobs with a transaction: lock and unlock for each write
 for (int i = 3; --i >= 0;)
 {
  const auto blob = client.transaction([argv](joedb::Writable_Journal &journal)
  {
   return journal.write_blob(argv[1]);
  });
  std::cout << "wrote blob with transaction: " << blob.get_position() << '\n';
  std::cout << "blob: " << server_file.read_blob(blob) << '\n';
 }

 return 0;
}

int main(int argc, char **argv)
{
 return joedb::main_exception_catcher(write_server_blob, argc, argv);
}
