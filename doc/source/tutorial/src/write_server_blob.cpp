#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/type_io.h"
#include "joedb/concurrency/Local_Connector.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/concurrency/Writable_Journal_Client.h"

#include <iostream>

/// Demonstration of joedb::Server_File
///
/// This demonstrates how to connect to a joedb server to read and write blobs,
/// without downloading a full replica of the database.
static int write_server_blob(joedb::Arguments &arguments)
{
 const std::string blob_string{arguments.get_next("<blob_string>")};

 if (arguments.missing())
 {
  arguments.print_help(std::cerr);
  std::cerr << "This program will try to connect to a local server.\n";
  std::cerr << "Before running this program, start a joedb server with:\n";
  std::cerr << "joedb_server blobs.joedb\n";
  std::cerr << "You can interactively read and write blobs this way:\n";
  std::cerr << "joedb_client --db none server local blobs.joedb.sock\n";
  return 1;
 }

 // Connect to the server
 joedb::Local_Connector connector("blobs.joedb.sock");
 joedb::Server_File server_file(connector, &std::cerr);

 // Creating the client: server file serves both as file and connection
 joedb::Writable_Journal_Client client(server_file, server_file);

 // Write blobs with a Client_Lock: keeps the server locked between writes
 {
  joedb::Writable_Journal_Client_Lock lock(client);

  for (int i = 3; --i >= 0;)
  {
   const joedb::Blob blob = lock.get_journal().write_blob(blob_string);
   lock.checkpoint_and_push();
   std::cout << "wrote blob with lock: ";
   joedb::write_blob(std::cout, blob);
   std::cout << "\nblob: " << server_file.read_blob(blob) << '\n';
  }

  lock.unlock();
 }

 // Write blobs with a transaction: lock and unlock for each write
 for (int i = 3; --i >= 0;)
 {
  const auto blob = client.transaction([&](joedb::Writable_Journal &journal)
  {
   return journal.write_blob(blob_string);
  });
  std::cout << "wrote blob with transaction: ";
  joedb::write_blob(std::cout, blob);
  std::cout << "\nblob: " << server_file.read_blob(blob) << '\n';
 }

 return 0;
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(write_server_blob, argc, argv);
}
