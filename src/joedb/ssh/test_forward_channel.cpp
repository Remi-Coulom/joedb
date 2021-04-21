#include "joedb/ssh/Forward_Channel.h"
#include "joedb/concurrency/network_integers.h"

#include <iostream>
#include <sstream>

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  void test(const char *user, const char *host, uint16_t port)
  ///////////////////////////////////////////////////////////////////////////
  {
   Session session(user, host, 22, 0);
   Forward_Channel channel(session, "localhost", port);

   char buffer[13];

   buffer[0] = 'j';
   buffer[1] = 'o';
   buffer[2] = 'e';
   buffer[3] = 'd';
   buffer[4] = 'b';

   to_network(12, buffer + 5);

   const size_t written = channel.write_some(buffer, 13);
   std::cout << "written = " << written << '\n';

   const size_t read = channel.read_some(buffer, 13);
   std::cout << "read = " << read << '\n';

   int64_t server_version = from_network(buffer + 5);
   std::cout << "server_version = " << server_version << '\n';
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc != 4)
 {
  std::cerr << "usage: " << argv[0] << " <user> <host> <port>\n";
  return 1;
 }
 else
 {
  const char *user = argv[1];
  const char *host = argv[2];
  uint16_t port;
  std::istringstream(argv[3]) >> port;

  try
  {
   joedb::ssh::test(user, host, port);
  }
  catch (const std::exception &e)
  {
   std::cerr << "Error: " << e.what() << '\n';
   return 1;
  }
 }

 return 0;
}
