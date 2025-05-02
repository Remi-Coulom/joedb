#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Writable_Client.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/ssh/Forward_Channel.h"

#include <iostream>

/// test how long we can wait without losing connection
int main(int argc, char **argv)
{
 if (argc < 4)
 {
  std::cerr << "usage: " << argv[0] << " <user> <host> <port>\n";
  return 1;
 }

 std::chrono::seconds wait(1);
 std::chrono::seconds increment(1);

 joedb::Memory_File file;
 joedb::Writable_Journal journal(file);

 while (true)
 {
  joedb::ssh::Session session(argv[1], argv[2], 22, 0);
  joedb::ssh::Forward_Channel channel
  (
   session,
   "localhost",
   uint16_t(std::stoi(argv[3]))
  );
  joedb::Server_Connection connection(channel, &std::cerr);
  joedb::Writable_Client client
  (
   journal,
   connection,
   joedb::Content_Check::none
  );

  try
  {
   client.pull(wait + increment);
   wait = wait + increment;
   increment *= 2;
  }
  catch (...)
  {
   increment = (std::chrono::seconds(3) + increment) / 4;
  }
 }

 return 0;
}
