#ifndef joedb_Portable_File_declared
#define joedb_Portable_File_declared

#include "joedb/journal/Stream_File.h"

#include <fstream> // IWYU pragma: keep

namespace joedb
{
 namespace detail
 {
  class Portable_File_Buffer
  {
   protected:
    std::filebuf filebuf;

   public:
    Portable_File_Buffer(const char *file_name, Open_Mode mode);
  };
 }

 /// @ingroup journal
 class Portable_File: private detail::Portable_File_Buffer, public Stream_File
 {
  public:
   Portable_File(const char *file_name, Open_Mode mode):
    detail::Portable_File_Buffer(file_name, mode),
    Stream_File(filebuf, mode)
   {
   }

   Portable_File(const std::string &file_name, Open_Mode mode):
    Portable_File(file_name.c_str(), mode)
   {
   }
 };
}

#endif
