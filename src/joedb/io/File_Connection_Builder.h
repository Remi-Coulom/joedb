#ifndef joedb_File_Connection_Builder_declared
#define joedb_File_Connection_Builder_declared

#include "joedb/io/Connection_Builder.h"
#include "joedb/io/File_Parser.h"
#include "joedb/concurrency/File_Connection.h"
#include "joedb/journal/File.h"

#include <iostream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class File_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  private:
   File_Parser file_parser;
   std::unique_ptr<Connection> connection;

  public:
   const char *get_name() const final {return "file";}
   int get_min_parameters() const final {return 1;}
   int get_max_parameters() const final {return 100;}

   const char *get_parameters_description() const final
   {
    return "<file>";
   }

   Connection &build(int argc, char **argv) final
   {
    int arg_index = 0;
    std::ostream null_stream(nullptr);
    file_parser.parse(null_stream, argc, argv, arg_index);

    connection.reset(new File_Connection(file_parser.get_file()));

    return *connection;
   }
 };
}

#endif
