#ifndef joedb_File_Connection_Builder_declared
#define joedb_File_Connection_Builder_declared

#include "joedb/ui/Connection_Builder.h"
#include "joedb/ui/File_Parser.h"
#include "joedb/concurrency/File_Connection.h"

#include <iostream>

namespace joedb
{
 /// @ingroup ui
 class File_Connection_Builder: public Connection_Builder
 {
  private:
   File_Parser file_parser;
   std::optional<Readonly_Journal> readonly_journal;
   std::optional<Writable_Journal> writable_journal;
   std::unique_ptr<Connection> connection;

  public:
   const char *get_name() const override {return "file";}

   std::string get_parameters_description() const override
   {
    return "<file>";
   }

   Connection *build(Arguments &arguments, Abstract_File *file) override
   {
    std::ostream null_stream(nullptr);

    if (file_parser.parse(null_stream, arguments))
    {
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
    }

    return connection.get();
   }
 };
}

#endif
