#ifndef joedb_CURL_File_declared
#define joedb_CURL_File_declared

#include "joedb/journal/Buffered_File.h"

#define CURL_NO_OLDIES
#include <curl/curl.h>

namespace joedb
{
 class CURL_Easy
 {
  protected:
   CURL * const curl;

  public:
   CURL_Easy();
   CURL_Easy(const CURL_Easy &) = delete;
   CURL_Easy &operator=(const CURL_Easy &) = delete;
   ~CURL_Easy();
 };

 /// @ingroup journal
 class CURL_File: private CURL_Easy, public Buffered_File
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

   void perform_range(int64_t start, int64_t size) const;

   size_t pread(char *buffer, size_t size, int64_t offset) const override;
   void copy_to(Buffered_File &destination, int64_t start, int64_t size) const override;

  public:
   CURL_File(const char *url, bool verbose);
 };
}

#endif
