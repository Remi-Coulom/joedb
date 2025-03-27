#ifndef joedb_Connection_Parser_declared
#define joedb_Connection_Parser_declared

#include "joedb/ui/Connection_Builder.h"

#include <vector>
#include <memory>
#include <iosfwd>

namespace joedb::ui
{
 ////////////////////////////////////////////////////////////////////////////
 class Connection_Parser
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::vector<std::unique_ptr<Connection_Builder>> builders;

   Connection_Builder &get_builder(const char *name) const;

   static concurrency::Pullonly_Connection &build
   (
    Connection_Builder &builder,
    int argc,
    char **argv,
    Buffered_File *file
   );

  public:
   Connection_Parser(bool local);

   void print_help(std::ostream &out) const;

   concurrency::Pullonly_Connection &build
   (
    int argc,
    char **argv,
    Buffered_File *file
   ) const;
 };
}

#endif
