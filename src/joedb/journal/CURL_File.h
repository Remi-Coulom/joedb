#ifndef joedb_CURL_File_declared
#define joedb_CURL_File_declared

#include "joedb/journal/Generic_File.h"

#include <curl/curl.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class CURL_File: public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   CURL * const curl;

   static void error_check(CURLcode code);

   struct pread_Callback_Data
   {
    char * const buffer;
    const size_t size;
    size_t offset;

    size_t copy(char *contents, size_t real_size);
   };

   static size_t pread_callback
   (
    void *contents,
    size_t size,
    size_t nmemb,
    void *p
   );

   static size_t copy_callback
   (
    void *contents,
    size_t size,
    size_t nmemb,
    void *p
   );

   void perform_range(int64_t start, int64_t size);

   size_t pread(char *buffer, size_t size, int64_t offset) final;
   void pwrite(const char *buffer, size_t size, int64_t offset) final;
   void copy_to(Generic_File &destination, int64_t start, int64_t size) final;

  public:
   CURL_File(const char *url, bool verbose);

   int64_t get_size() const final;

   ~CURL_File();
 };
}

#endif
