#ifndef joedb_Dump_Connection_declared
#define joedb_Dump_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/io/Interpreter_Dump_Writable.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Dump_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Interpreter_Dump_Writable dump{std::cout};
   bool muted;

  public:
   Dump_Connection(bool muted): muted(muted) {}

   int64_t handshake(Readonly_Journal &client_journal) final
   {
    const int64_t client_position = client_journal.get_position();

    dump.set_muted(muted);
    client_journal.replay_log(dump);
    dump.set_muted(false);

    client_journal.set_position(client_position);

    return client_journal.get_checkpoint_position();
   }

   int64_t push
   (
    Readonly_Journal &client_journal,
    int64_t server_position,
    bool unlock_after
   ) final
   {
    const int64_t client_position = client_journal.get_position();
    client_journal.set_position(server_position);
    client_journal.play_until_checkpoint(dump);
    int64_t new_server_position = client_journal.get_position();
    client_journal.set_position(client_position);
    return new_server_position;
   }
 };
}

#endif
