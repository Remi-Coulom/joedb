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

   static void throw_if_error(CURLcode code);

   struct Callback_Data
   {
    char * const buffer;
    const size_t size;
    size_t offset;

    size_t copy(char *contents, size_t real_size);
   };

   static size_t callback(void *contents, size_t size, size_t nmemb, void *p);

   size_t pread(char *buffer, size_t size, int64_t offset) final;
   void pwrite(const char *buffer, size_t size, int64_t offset) final;

  public:
   CURL_File(const char *url, bool verbose);

   int64_t get_size() const final;

   ~CURL_File();
 };
}

#endif
