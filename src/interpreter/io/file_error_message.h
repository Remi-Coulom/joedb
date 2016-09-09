#ifndef joedb_file_error_message_declared
#define joedb_file_error_message_declared

#include <iosfwd>

namespace joedb
{
 class File;

 int file_error_message(std::ostream &out, const File &file);
}

#endif
