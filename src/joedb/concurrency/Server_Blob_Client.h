#ifndef Server_Blob_Client_declared
#define Server_Blob_Client_declared

#include "joedb/concurrency/Server_Client.h"
#include "joedb/Blob.h"

namespace joedb
{
 class Server_Blob_Client: public Server_Client, public Blob_Reader
 {
  public:
   Server_Blob_Client(Channel &channel, std::ostream *log);
   std::string read_blob_data(Blob blob) override;
 };
}

#endif
