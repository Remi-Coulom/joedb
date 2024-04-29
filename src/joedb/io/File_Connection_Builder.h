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
 class File_Construction
 /////////////////////////////////////////////////////////////////////////////
 {
  protected:
   File_Parser file_parser;

  public:
   File_Construction(int argc, char **argv)
   {
    int arg_index = 0;
    file_parser.parse(std::cout, argc, argv, arg_index);
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 class File_Connection_Data: public File_Construction, public File_Connection
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   File_Connection_Data(int argc, char **argv):
    File_Construction(argc, argv),
    File_Connection(file_parser.get_file())
   {
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 class File_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   const char *get_name() const final {return "file";}
   int get_min_parameters() const final {return 1;}
   int get_max_parameters() const final {return 100;}

   const char *get_parameters_description() const final
   {
    return "<file>";
   }

   std::unique_ptr<Connection> build(int argc, char **argv) final
   {
    return std::unique_ptr<Connection>
    (
     new File_Connection_Data(argc, argv)
    );
   }
 };
}

#endif
