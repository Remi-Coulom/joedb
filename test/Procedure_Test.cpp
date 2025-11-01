#include "joedb/error/CLog_Logger.h"
#include "joedb/rpc/Server.h"
#include "joedb/concurrency/Local_Channel.h"

#include "tutorial/Client.h"
#include "tutorial/rpc/Procedures.h"
#include "tutorial/rpc/Signatures.h"
#include "tutorial/rpc/Client.h"

#include <gtest/gtest.h>

namespace joedb
{
 static CLog_Logger logger;
 static constexpr int log_level = 0;
 static const char * const endpoint_path = "test.rpc.sock";

 class Test_Procedure_Server
 {
  public:
   Memory_File file;
   tutorial::Client client;
   tutorial::rpc::Service service;
   tutorial::rpc::Procedures procedures;
   rpc::Server server;

  public:
   Test_Procedure_Server():
    client(file),
    service(client),
    procedures(service),
    server
    (
     logger,
     log_level,
     1,
     endpoint_path,
     tutorial::rpc::get_signatures(),
     procedures.procedures
    )
   {
   }

   void stop()
   {
    server.stop();
   }
 };

 TEST(Procedure, simple)
 {
  Test_Procedure_Server server;
  Local_Channel channel(endpoint_path);
  tutorial::rpc::Client rpc_client(channel);

  {
   tutorial::rpc::city::Memory_Database city;
   city.set_name("Paris");
   EXPECT_EQ(server.client.get_database().get_city_table().get_size(), 0);
   rpc_client.insert_city(city);
   EXPECT_EQ(server.client.get_database().get_city_table().get_size(), 1);
   rpc_client.delete_city(city);
   EXPECT_EQ(server.client.get_database().get_city_table().get_size(), 0);
  }

  server.stop();
 }

 TEST(Procedure, exception)
 {
  Test_Procedure_Server server;
  Local_Channel channel(endpoint_path);
  tutorial::rpc::Client rpc_client(channel);

  {
   tutorial::rpc::city::Memory_Database city;
   city.set_name("Paris");
   rpc_client.insert_city(city);
   try
   {
    rpc_client.insert_city(city);
    FAIL() << "This should have thrown";
   }
   catch (joedb::Exception &e)
   {
    EXPECT_EQ(e.what(), std::string("city already exists"));
   }
  }

  server.stop();
 }
}
