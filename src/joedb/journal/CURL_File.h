#ifndef joedb_CURL_File_declared
#define joedb_CURL_File_declared

#include "joedb/journal/Generic_File.h"

#define CURL_NO_OLDIES
#include <curl/curl.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class CURL_Easy
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   CURL * const curl;

  public:
   CURL_Easy();
   CURL_Easy(const CURL_Easy &) = delete;
   CURL_Easy &operator=(const CURL_Easy &) = delete;
   ~CURL_Easy();
 };

 ////////////////////////////////////////////////////////////////////////////
 class CURL_File: public CURL_Easy, public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
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

   size_t pread(char *buffer, size_t size, int64_t offset) override;
   void copy_to(Generic_File &destination, int64_t start, int64_t size) override;

  public:
   CURL_File(const char *url, bool verbose);
 };
}

#endif
