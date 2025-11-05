#ifndef joedb_rpc_Client_declared
#define joedb_rpc_Client_declared

#include "joedb/rpc/Signature.h"
#include "joedb/rpc/get_hash.h"
#include "joedb/concurrency/Channel.h"
#include "joedb/concurrency/Keep_Alive_Thread.h"
#include "joedb/Thread_Safe.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Buffer.h"
#include "joedb/error/Exception.h"

#include <vector>

namespace joedb::rpc
{
 class Client: public Ping_Client
 {
  private:
   Buffer<13> buffer;
   Thread_Safe<Channel&> channel;
   const std::vector<Signature> &signatures;

   int64_t session_id;

   void handshake()
   {
    const SHA_256::Hash h = get_hash(signatures);

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

   Thread_Safe<Channel&> &get_channel() override {return channel;}
   void locked_ping(Lock<Channel&> &lock) override
   {
    char c = 'P';
    lock->write(&c, 1);
    lock->read(&c, 1);
   }
   Keep_Alive_Thread keep_alive;

  public:
   Client
   (
    Channel &channel,
    const std::vector<Signature> &signatures,
    std::chrono::milliseconds keep_alive_interval = std::chrono::milliseconds(0)
   ):
    channel(channel),
    signatures(signatures),
    keep_alive(*this, keep_alive_interval)
   {
    handshake();
   }

   void call(int64_t procedure_id, Memory_File &file)
   {
    Lock<Channel&> lock(channel);

    auto &signature = signatures[procedure_id];

    {
     // Check that the prolog is matching?
     const int64_t from = int64_t(signature.prolog.size());
     const int64_t until = file.get_size();

     buffer.index = 0;
     buffer.write<char>('C');
     buffer.write<int64_t>(procedure_id);
     buffer.write<int64_t>(until);

     // Could be optimized into one single write?
     lock->write(buffer.data, buffer.index);
     lock->write(file.get_data().data() + from, until - from);
     lock->read(buffer.data, 9);
    }

    buffer.index = 0;
    const char reply = buffer.read<char>();

    if (reply == 'C')
    {
     const size_t from = file.get_data().size();
     const int64_t until = buffer.read<int64_t>();
     file.get_data().resize(size_t(until));
     lock->read(file.get_data().data() + from, size_t(until) - from);
     file.pwrite((const char *)&until, 8, 0);
     file.pwrite((const char *)&until, 8, 8);
    }
    else
    {
     const int64_t n = buffer.read<int64_t>();
     std::string error_message;
     error_message.resize(n);
     lock->read(error_message.data(), n);
     throw Exception(error_message);
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
