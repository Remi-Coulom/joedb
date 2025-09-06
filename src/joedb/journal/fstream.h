#ifndef joedb_fstream_declared
#define joedb_fstream_declared

#include "joedb/journal/File.h"
#include "joedb/journal/streambuf.h"

#include <iostream>
#include <istream>
#include <ostream>

namespace joedb
{
 namespace detail
 {
  struct fstream_Parent
  {
   protected:
    File file;
    streambuf streambuf;

   public:
    fstream_Parent(const char *file_name, Open_Mode mode):
     file(file_name, mode),
     streambuf(file)
    {
    }
  };
 }

 class fstream: private detail::fstream_Parent, public std::iostream
 {
  public:
   fstream(const char *file_name, Open_Mode mode):
    fstream_Parent(file_name, mode),
    std::iostream(&streambuf)
   {
   }

   fstream(const std::string &file_name, Open_Mode mode):
    fstream(file_name.c_str(), mode)
   {
   }
 };

 class ifstream: private detail::fstream_Parent, public std::istream
 {
  public:
   ifstream(const char *file_name, Open_Mode mode):
    fstream_Parent(file_name, mode),
    std::istream(&streambuf)
   {
   }

   ifstream(const std::string &file_name, Open_Mode mode):
    ifstream(file_name.c_str(), mode)
   {
   }
 };

 class ofstream: private detail::fstream_Parent, public std::ostream
 {
  public:
   ofstream(const char *file_name, Open_Mode mode):
    fstream_Parent(file_name, mode),
    std::ostream(&streambuf)
   {
   }

   ofstream(const std::string &file_name, Open_Mode mode):
    ofstream(file_name.c_str(), mode)
   {
   }
 };
}

#endif
