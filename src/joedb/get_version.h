#ifndef joedb_get_version_declared
#define joedb_get_version_declared

namespace joedb
{
 const char *get_version()
 {
  return
#include "joedb/../../VERSION"
  ;
 }
}

#endif
