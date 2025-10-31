#ifndef joedb_Connection_Parser_declared
#define joedb_Connection_Parser_declared

#include "joedb/ui/Connection_Builder.h"
#include "joedb/error/Logger.h"

#include <vector>
#include <memory>
#include <iosfwd>

namespace joedb
{
 /// Create an instance of a @ref Connection by parsing command-line arguments
 /// @ingroup ui
 class Connection_Parser
 {
  private:
   std::vector<std::unique_ptr<Connection_Builder>> builders;

   Connection_Builder &get_builder(std::string_view name) const;

  public:
   Connection_Parser();

   void print_help(std::ostream &out) const;

   Connection *build
   (
    Logger &logger,
    Arguments &arguments,
    Abstract_File *file
   ) const;
 };
}

#endif
