#ifndef joedb_client_main_declared
#define joedb_client_main_declared

namespace joedb::ui
{
 class Connection_Builder;

 int client_main(int argc, char **argv, Connection_Builder &&builder);
}

#endif
