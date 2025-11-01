#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Writable_Client.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/ssh/Forward_Channel.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/Parsed_Logger.h"

#include <iostream>

namespace joedb
{
 /// test how long we can wait without losing connection
 static int keepalive_test(Arguments &arguments)
 {
  Parsed_Logger logger(arguments);

  const std::string_view user = arguments.get_next("user");
  const std::string_view host = arguments.get_next("host");
  const std::string_view endpoint_path = arguments.get_next("endpoint_path");

  if (arguments.missing())
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  std::chrono::seconds wait(1);
  std::chrono::seconds increment(1);

  Memory_File file;
  Writable_Journal journal(file);

  while (true)
  {
   ssh::Session session(user.data(), host.data(), 22, 0);
   ssh::Forward_Channel channel(session, endpoint_path.data());
   Server_Connection connection(channel, &logger.get());
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
 return joedb::main_wrapper(joedb::keepalive_test, argc, argv);
}
