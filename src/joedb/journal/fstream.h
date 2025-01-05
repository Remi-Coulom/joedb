#ifndef joedb_fstream_declared
#define joedb_fstream_declared

#include "joedb/journal/File.h"
#include "joedb/journal/filebuf.h"

#include <iostream>

namespace joedb
{
 class iostream: public std::iostream
 {
  public:
   iostream(joedb::filebuf &buf): std::iostream(&buf) {}
 };

 class fstream_data
 {
  protected:
   File file;
   filebuf buf;

  public:
   fstream_data(const char *file_name, Open_Mode open_mode):
    file(file_name, open_mode),
    buf(file)
   {
   }
 };

 class fstream: private fstream_data, public joedb::iostream
 {
  public:
   fstream(const char *file_name, Open_Mode open_mode):
    fstream_data(file_name, open_mode),
    joedb::iostream(buf)
   {
   }
 };

 class ofstream: private fstream_data, public std::ostream
 {
  public:
   ofstream
   (
    const char *file_name,
    Open_Mode open_mode = Open_Mode::create_new
   ):
    fstream_data(file_name, open_mode),
    std::ostream(&buf)
   {
   }
 };

 class ifstream: private fstream_data, public std::istream
 {
  public:
   ifstream(const char *file_name):
    fstream_data(file_name, Open_Mode::read_existing),
    std::istream(&buf)
   {
   }
 };

 class dummy_stream: public std::iostream
 {
 };
}

#endif
