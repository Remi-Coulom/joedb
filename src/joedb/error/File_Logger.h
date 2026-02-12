#ifndef joedb_File_Logger_declared
#define joedb_File_Logger_declared

#include "joedb/error/Stream_Logger.h"

#include <fstream>

namespace joedb
{
 namespace detail
 {
  class File_Logger_Data
  {
   public:
    std::ofstream ofs;
    File_Logger_Data(const std::string &file_name):
     ofs(file_name, std::ios::out | std::ios::trunc)
    {
    }
  };
 }

 /// @ingroup error
 class File_Logger: private detail::File_Logger_Data, public Stream_Logger
 {
  private:

  public:
   File_Logger(const std::string &file_name):
    File_Logger_Data(file_name),
    Stream_Logger(ofs)
   {
   }
 };
}

#endif
