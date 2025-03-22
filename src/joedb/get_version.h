#ifndef joedb_get_version_declared
#define joedb_get_version_declared

namespace joedb
{
 constexpr const char *get_version()
 {
  return
#include "joedb/../../VERSION"
  ;
 }

 constexpr int get_version_int() {return JOEDB_VERSION;}
}

#endif
