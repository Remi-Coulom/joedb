#include "joedb/concurrency/Network_Connector.h"
#include "joedb/concurrency/Server_File.h"
#include "joedb/concurrency/Writable_Journal_Client.h"

#include <iostream>

int main()
{
 joedb::Network_Connector connector("localhost", "1234");
 joedb::Server_File server_file(connector);
 joedb::Writable_Journal_Client client(server_file, server_file);

 const auto blob = client.transaction([](joedb::Writable_Journal &journal)
 {
  return journal.write_blob("blob");
 });

 std::cout << server_file.read_blob(blob) << '\n';

 return 0;
}
