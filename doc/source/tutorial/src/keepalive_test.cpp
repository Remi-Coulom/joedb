#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Writable_Client.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/ssh/Forward_Channel.h"
#include "joedb/ui/main_exception_catcher.h"

#include <iostream>

namespace joedb
{
 /// test how long we can wait without losing connection
 static int keepalive_test(int argc, char **argv)
 {
  if (argc < 4)
  {
   std::cerr << "usage: " << argv[0] << " <user> <host> <endpoint_path>\n";
   return 1;
  }

  std::chrono::seconds wait(1);
  std::chrono::seconds increment(1);

  Memory_File file;
  Writable_Journal journal(file);

  while (true)
  {
   ssh::Session session(argv[1], argv[2], 22, 0);
   ssh::Forward_Channel channel(session, argv[3]);
   Server_Connection connection(channel, &std::cerr);
   Writable_Client client(journal, connection, Content_Check::none);

   try
   {
    channel.set_timeout(wait + increment + std::chrono::seconds(30));
    client.pull(wait + increment);
    wait = wait + increment;
    increment *= 2;
   }
   catch (const std::exception &e)
   {
    std::cerr << "Caught exception: " << e.what() << '\n';
    increment = (std::chrono::seconds(3) + increment) / 4;
   }
  }

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_exception_catcher(joedb::keepalive_test, argc, argv);
}
