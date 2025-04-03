#include "joedb/ui/main_exception_catcher.h"
#include "joedb/concurrency/Network_Channel.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/concurrency/Journal_Client.h"

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
  std::cerr << "Before running it, start a joedb server with:\n";
  std::cerr << "joedb_server -port <port> blobs.joedb\n";
  std::cerr << "You can interactively read and write blobs this way:\n";
  std::cerr << "joedb_client server network localhost <port>\n";
  return 1;
 }

 // Connect to the server
 joedb::Network_Channel channel("localhost", argv[1]);
 joedb::Server_File server_file(channel);

 // Creating the client: server file serves both as file and connection
 joedb::Journal_Client client(server_file, server_file);

 // Writing a new blob
 const joedb::Blob blob = client.write_blob_data(argv[2]);

 // Reading it back (no need of client, can be read from file directly)
 std::cout << "blob: " << server_file.read_blob_data(blob) << '\n';
 std::cout << "blob position: " << blob.get_position() << '\n';

 return 0;
}

int main(int argc, char **argv)
{
 return joedb::main_exception_catcher(write_server_blob, argc, argv);
}
