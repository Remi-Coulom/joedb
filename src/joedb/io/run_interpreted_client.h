#ifndef joedb_run_interpreted_client_declared
#define joedb_run_interpreted_client_declared

namespace joedb
{
 class Interpreted_Client;
 class Connection;

 void run_interpreted_client(Interpreted_Client &client);
 void run_interpreted_client(Connection &connection, const char *file_name);
}

#endif
