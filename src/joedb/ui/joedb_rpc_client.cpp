#include "joedb/ui/main_wrapper.h"
#include "joedb/concurrency/Local_Channel.h"
#include "../../doc/source/tutorial/src/tutorial/Procedures.h"

#include <iostream>

namespace joedb
{
 static int rpc_client(Arguments &arguments)
 {
  const std::string_view endpoint_path = arguments.get_next("<endpoint_path>");
  if (arguments.missing())
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  Memory_File file;
  tutorial::Client client(file);
  tutorial::Procedures procedures(client);

  Local_Channel channel((std::string(endpoint_path)));

  const SHA_256::Hash h = procedures.get_hash();
  channel.write((const char *)h.data(), h.size() * sizeof h[0]);

  Buffer<13> buffer;
  channel.read(buffer.data, 9);
  buffer.index = 0;

  {
   const char H = buffer.read<char>();
   std::cerr << "handshake: " << H << '\n';
  }

  const int64_t session_id = buffer.read<int64_t>();
  std::cerr << "session_id = " << session_id << '\n';

  channel.write("Q", 1);

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::rpc_client, argc, argv);
}
