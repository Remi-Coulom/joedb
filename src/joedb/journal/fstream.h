#ifndef joedb_fstream_declared
#define joedb_fstream_declared

#include "joedb/journal/File.h"
#include "joedb/journal/filebuf.h"

#include <iostream>

namespace joedb
{
 class dummy_stream: public std::iostream
 {
 };

 class iostream: public std::iostream
 {
  private:
   joedb::filebuf &buf;

  public:
   iostream(joedb::filebuf &buf): std::iostream(&buf), buf(buf) {}
   joedb::filebuf &get_filebuf() {return buf;}
 };

 class fstream_data
 {
  protected:
   File file;
   filebuf data_buf;

  public:
   fstream_data(const char *file_name, Open_Mode open_mode):
    file(file_name, open_mode),
    data_buf(file)
   {
   }
 };

 class fstream: private fstream_data, public joedb::iostream
 {
  public:
   fstream(const char *file_name, Open_Mode open_mode):
    fstream_data(file_name, open_mode),
    joedb::iostream(data_buf)
   {
   }
 };

 class ifstream: public fstream
 {
  public:
   ifstream(const char *file_name):
    fstream(file_name, Open_Mode::read_existing)
   {
   }
 };
}

#endif
