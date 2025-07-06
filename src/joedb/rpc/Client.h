#ifndef joedb_rpc_Client_declared
#define joedb_rpc_Client_declared

#include "joedb/rpc/Procedures.h"
#include "joedb/concurrency/Channel.h"
#include "joedb/Thread_Safe.h"
#include "joedb/journal/Memory_File.h"

namespace joedb::rpc
{
 class Client
 {
  private:
   Buffer<13> buffer;
   Thread_Safe<Channel&> channel;
   Procedures procedures;

   int64_t session_id;

   void handshake()
   {
    const SHA_256::Hash h = procedures.get_hash();

    {
     Lock<Channel&> lock(channel);
     lock->write((const char *)h.data(), h.size() * sizeof h[0]);
     lock->read(buffer.data, 9);
    }

    buffer.index = 0;
    if (buffer.read<char>() != 'H')
     throw Exception("failed handshake");
    session_id = buffer.read<int64_t>();
   }

   void ping(Lock<Channel&> &lock)
   {
    char c = 'P';
    lock->write(&c, 1);
    lock->read(&c, 1);
   }

   // TODO: ping thread

  public:
   Client(Channel &channel, Procedures &procedures):
    channel(channel),
    procedures(procedures)
   {
    handshake();
   }

   void call(int64_t procedure_id, Memory_File &file)
   {
    Lock<Channel&> lock(channel);

    auto &procedure = *procedures.get_procedures()[procedure_id];

    {
     const int64_t from = int64_t(procedure.get_prolog().size());
     const int64_t until = file.get_size();

     buffer.index = 0;
     buffer.write<char>('C');
     buffer.write<int64_t>(procedure_id);
     buffer.write<int64_t>(until);

     lock->write(buffer.data, buffer.index);
     lock->write(file.get_data().data() + from, until - from);
     lock->read(buffer.data, 9);
    }

    buffer.index = 0;
    const char reply = buffer.read<char>();

    if (reply == 'C')
    {
     const size_t from = file.get_data().size();
     const size_t until = static_cast<size_t>(buffer.read<int64_t>());
     file.get_data().resize(until);
     lock->read(file.get_data().data() + from, until - from);
     file.set_position(0);
     file.write<int64_t>(until);
     file.write<int64_t>(until);
     file.set_position(from);
    }
    else
    {
     const int64_t n = buffer.read<int64_t>();
     lock->read(buffer.data, n);
    }
   }

   ~Client()
   {
    try
    {
     Lock<Channel&> lock(channel);
     lock->write("Q", 1);
    }
    catch (...)
    {
    }
   }
 };
}

#endif
