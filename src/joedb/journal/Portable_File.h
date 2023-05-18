#ifndef joedb_Portable_File_declared
#define joedb_Portable_File_declared

#include "joedb/journal/Stream_File.h"

#include <fstream>

namespace joedb
{
 ///////////////////////////////////////////////////////////////////////////
 class Portable_File_Buffer
 ///////////////////////////////////////////////////////////////////////////
 {
  private:
   bool try_open(const char *file_name, Open_Mode mode);

  protected:
   std::filebuf filebuf;
   Open_Mode actual_mode;

  public:
   Portable_File_Buffer(const char *file_name, Open_Mode mode);
 };

 ///////////////////////////////////////////////////////////////////////////
 class Portable_File: private Portable_File_Buffer, public Stream_File
 ///////////////////////////////////////////////////////////////////////////
 {
  public:
   Portable_File(const char *file_name, Open_Mode mode):
    Portable_File_Buffer(file_name, mode),
    Stream_File(filebuf, mode)
   {
    set_mode(actual_mode);
   }

   Portable_File(const std::string &file_name, Open_Mode mode):
    Portable_File(file_name.c_str(), mode)
   {
   }
 };
}

#endif
