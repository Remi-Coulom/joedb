#include "joedb/get_version.h"

namespace joedb
{
 const char *get_version()
 {
  return
#include "joedb/../../VERSION"
  ;
 }
}
