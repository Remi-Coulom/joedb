#ifndef joedb_Interpreted_File_declared
#define joedb_Interpreted_File_declared

#include "joedb/journal/Readonly_Interpreted_File.h"
#include "joedb/journal/fstream.h"

namespace joedb
{
 /// @ingroup journal
 class Interpreted_Stream_File: public Readonly_Interpreted_File
 {
  private:
   std::iostream &stream;

   void pwrite(const char *buffer, size_t size, int64_t offset) override;

  public:
   Interpreted_Stream_File(std::iostream &stream);
 };

 namespace detail
 {
  class Interpreted_File_Data
  {
   protected:
    joedb::fstream file_stream;

   public:
    Interpreted_File_Data(const char *file_name, Open_Mode mode);
    ~Interpreted_File_Data();
  };
 }

 /// Read or write to a text file in joedbi format
 ///
 /// This class does not provide any handling of concurrency or durability
 ///
 /// @ingroup journal
 class Interpreted_File:
  private detail::Interpreted_File_Data,
  public Interpreted_Stream_File
 {
  public:
   Interpreted_File(const char *file_name, Open_Mode mode);
   Interpreted_File(const std::string &file_name, Open_Mode mode):
    Interpreted_File(file_name.data(), mode)
   {
   }
 };
}

#endif
