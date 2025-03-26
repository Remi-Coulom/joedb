#ifndef joedb_File_Connection_Builder_declared
#define joedb_File_Connection_Builder_declared

#include "joedb/ui/Connection_Builder.h"
#include "joedb/ui/File_Parser.h"
#include "joedb/concurrency/File_Connection.h"

#include <iostream>

namespace joedb::ui
{
 /////////////////////////////////////////////////////////////////////////////
 class File_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  private:
   File_Parser file_parser;
   std::optional<Readonly_Journal> readonly_journal;
   std::optional<Writable_Journal> writable_journal;
   std::unique_ptr<Pullonly_Connection> connection;

  public:
   const char *get_name() const final {return "file";}
   int get_min_parameters() const final {return 1;}
   int get_max_parameters() const final {return 100;}

   const char *get_parameters_description() const final
   {
    return "<file>";
   }

   Pullonly_Connection &build(int argc, char **argv) final
   {
    int arg_index = 0;
    std::ostream null_stream(nullptr);
    file_parser.parse(null_stream, argc, argv, arg_index);

    if (file_parser.get_file()->is_readonly())
    {
     readonly_journal.emplace(*file_parser.get_file());
     connection.reset(new Pullonly_Journal_Connection(*readonly_journal));
    }
    else
    {
     writable_journal.emplace(*file_parser.get_file());
     connection.reset(new Journal_Connection(*writable_journal));
    }

    return *connection;
   }
 };
}

#endif
