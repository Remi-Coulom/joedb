#include "file_error_message.h"
#include "File.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int joedb::file_error_message(std::ostream &out, const File &file)
/////////////////////////////////////////////////////////////////////////////
{
 if (file.get_status() == joedb::File::status_t::locked)
 {
  out << "joedb file error: locked by another process\n";
  return 1;
 }
 else if (file.get_status() != joedb::File::status_t::success)
 {
  out << "joedb file error: could not open\n";
  return 1;
 }
 return 0;
}
