#include "../tutorial.cpp"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/journal/Readonly_Journal.cpp"
#include "joedb/journal/Writable_Journal.cpp"
#include "joedb/journal/Posix_File.cpp"
#include "joedb/Writable.cpp"
#include "joedb/journal/Memory_File.cpp"
#include "joedb/io/type_io.cpp"
#include "joedb/journal/Generic_File.cpp"
#include "joedb/Destructor_Logger.cpp"
#include "external/wide_char_display_width.cpp"
#include "joedb/interpreter/Database.cpp"
#include "joedb/interpreter/Database_Schema.cpp"
#include "joedb/interpreter/Table.cpp"
#include "joedb/is_identifier.cpp"
#include "joedb/journal/Interpreted_File.cpp"
#include "joedb/Multiplexer.cpp"
#include "joedb/io/Interpreter.cpp"
#include "joedb/io/SQL_Dump_Writable.cpp"
#include "joedb/io/Interpreter_Dump_Writable.cpp"
#include "joedb/io/get_time_string.cpp"
#include "joedb/journal/diagnostics.cpp"
#include "joedb/get_version.cpp"
#include "joedb/io/dump.cpp"
#include "joedb/io/json.cpp"
#include "joedb/io/base64.cpp"
#include "joedb/Readable.cpp"

/////////////////////////////////////////////////////////////////////////////
static int local_concurrency(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 tutorial::Local_Client client("local_concurrency.joedb");

 client.transaction([](tutorial::Generic_File_Database &db)
 {
  db.new_city("Tokyo");
  db.new_city("New York");
  db.new_city("Paris");
 });

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(local_concurrency, argc, argv);
}
