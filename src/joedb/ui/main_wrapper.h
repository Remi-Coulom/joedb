#ifndef joedb_main_wrapper_declared
#define joedb_main_wrapper_declared

#include "joedb/ui/Arguments.h"

namespace joedb
{
 int main_wrapper(int (*main)(Arguments &), int argc, char **argv);
}

#endif
