#ifndef joedb_Interpreted_File_declared
#define joedb_Interpreted_File_declared

#include "joedb/journal/Readonly_Interpreted_File.h"
#include "joedb/journal/File_View.h"

namespace joedb
{
 /// @ingroup journal
 class Interpreted_Stream_File: public Readonly_Interpreted_File
 {
  private:
   std::iostream &stream;
   File_View file_view;
   Readonly_Journal readonly_journal;

   void pwrite(const char *buffer, size_t size, int64_t offset) override;

  public:
   Interpreted_Stream_File(std::iostream &stream);
 };

 namespace detail
 {
  class Interpreted_File_Data
  {
   protected:
    std::fstream file_stream;

   public:
    Interpreted_File_Data(const char *file_name);
    ~Interpreted_File_Data();
  };
 }

 /// @ingroup journal
 class Interpreted_File:
  private detail::Interpreted_File_Data,
  public Interpreted_Stream_File
 {
  public:
   Interpreted_File(const char *file_name);
 };
}

#endif
