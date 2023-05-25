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
    dump.set_muted(muted);
    client_journal.replay_log(dump);
    dump.set_muted(false);

    return client_journal.get_checkpoint_position();
   }

   void push
   (
    Readonly_Journal &client_journal,
    int64_t server_position,
    bool unlock_after
   ) final
   {
    client_journal.play_until_checkpoint(dump);
   }
 };
}

#endif
