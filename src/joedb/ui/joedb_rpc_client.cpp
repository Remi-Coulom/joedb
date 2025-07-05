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

  Memory_File client_file;
  tutorial::Client client(client_file);
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

  const int64_t procedure_id = 0;
  auto &procedure = *procedures.get_procedures()[procedure_id];

  tutorial::procedures::city::Memory_Database city;
  city.set_name("Tombouctou");
  city.soft_checkpoint();

  const int64_t from = int64_t(procedure.get_prolog().size());
  const int64_t until = city.get_journal().get_position();

  std::cerr << "from = " << from << '\n';
  std::cerr << "until = " << until << '\n';

  buffer.index = 0;
  buffer.write<char>('P');
  buffer.write<int64_t>(procedure_id);
  buffer.write<int64_t>(until);

  channel.write(buffer.data, buffer.index);
  channel.write(city.data() + from, until - from);

  std::cerr << "wrote " << until - from << " bytes\n";

  channel.read(buffer.data, 9);
  buffer.index = 0;
  const char reply = buffer.read<char>();

  if (reply == 'P')
  {
   const int64_t reply_until = buffer.read<int64_t>();
   std::cerr << "reply_until = " << reply_until << '\n';
  }
  else
  {
   const int64_t n = buffer.read<int64_t>();
   channel.read(buffer.data, n);
   std::cerr << "error: " << std::string_view(buffer.data, n) << '\n';
  }

  channel.write("Q", 1);

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::rpc_client, argc, argv);
}
