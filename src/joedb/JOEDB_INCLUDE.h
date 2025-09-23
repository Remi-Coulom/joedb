#ifndef JOEDB_INCLUDE
#define JOEDB_INCLUDE_STRINGIFY(s) #s
#define JOEDB_INCLUDE_STRINGIFY2(name,extension) JOEDB_INCLUDE_STRINGIFY(name.extension)
#define JOEDB_INCLUDE(name,extension) JOEDB_INCLUDE_STRINGIFY2(name,extension)
#endif
