#ifndef joedb_Connection_Builder_declared
#define joedb_Connection_Builder_declared

#include "joedb/ui/Arguments.h"

namespace joedb
{
 class Connection;
 class Buffered_File;

 /// @ingroup ui
 class Connection_Builder
 {
  public:
   virtual bool has_sharing_option() const {return false;}
   virtual bool get_default_sharing() const {return false;}
   virtual const char *get_name() const {return "";}
   virtual const char *get_parameters_description() const {return "";}

   virtual Connection *build(Arguments &arguments, Buffered_File *file) = 0;

   virtual ~Connection_Builder() = default;
 };
}

#endif
