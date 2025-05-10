#ifndef joedb_main_exception_catcher_declared
#define joedb_main_exception_catcher_declared

#include "joedb/ui/Arguments.h"

namespace joedb
{
 /// @ingroup ui
 int main_exception_catcher(int (*main)(Arguments &), int argc, char **argv);
}

#endif
