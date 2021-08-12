#ifndef joedb_Shared_Local_File_declared
#define joedb_Shared_Local_File_declared

#include "joedb/journal/Generic_File.h"

#include <memory>
#include <string>

namespace joedb
{
 class Mutex;

 ////////////////////////////////////////////////////////////////////////////
 class Shared_Local_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::unique_ptr<joedb::Generic_File> file;

  public:
   Shared_Local_File(Mutex &mutex, const std::string &file_name);

   joedb::Generic_File &get_file() const {return *file;}
 };
}

#endif
