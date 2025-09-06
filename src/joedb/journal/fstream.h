#ifndef joedb_fstream_declared
#define joedb_fstream_declared

#include "joedb/journal/File.h"
#include "joedb/journal/streambuf.h"

#include <iostream>

namespace joedb
{
 class fstream: private File, private streambuf, public std::iostream
 {
  public:
   fstream(const char *file_name, Open_Mode mode):
    File(file_name, mode),
    joedb::streambuf(*static_cast<File *>(this)),
    std::iostream(static_cast<std::streambuf *>(this))
   {
   }

   fstream(const std::string &file_name, Open_Mode mode):
    fstream(file_name.c_str(), mode)
   {
   }
 };
}

#endif
