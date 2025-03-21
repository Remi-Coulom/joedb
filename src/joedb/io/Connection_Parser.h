#ifndef joedb_Connection_Parser_declared
#define joedb_Connection_Parser_declared

#include "joedb/io/Connection_Builder.h"

#include <vector>
#include <memory>
#include <iosfwd>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Connection_Parser
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::vector<std::unique_ptr<Connection_Builder>> builders;

   Connection_Builder &get_builder(const char *name) const;

   static Pullonly_Connection &build
   (
    Connection_Builder &builder,
    int argc,
    char **argv
   );

  public:
   Connection_Parser(bool local, bool use_server_file);

   void print_help(std::ostream &out) const;

   Pullonly_Connection &build(int argc, char **argv) const;
 };
}

#endif
