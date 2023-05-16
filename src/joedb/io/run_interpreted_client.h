#ifndef joedb_run_interpreted_client_declared
#define joedb_run_interpreted_client_declared

namespace joedb
{
 class Client;
 class Connection;

 void run_interpreted_client(Client &client);
 void run_interpreted_client(Connection &connection, const char *file_name);
}

#endif
