#ifndef joedb_Connection_Builder_declared
#define joedb_Connection_Builder_declared

#include "joedb/ui/Arguments.h"
#include "joedb/error/Logger.h"

namespace joedb
{
 class Connection;
 class Abstract_File;

 /// @ingroup ui
 class Connection_Builder
 {
  public:
   virtual bool get_default_sharing() const {return false;}
   virtual const char *get_name() const {return "";}
   virtual std::string get_parameters_description() const {return "";}

   virtual Connection *build
   (
    Logger &logger,
    Arguments &arguments,
    Abstract_File *file
   ) = 0;

   virtual ~Connection_Builder() = default;
 };
}

#endif
