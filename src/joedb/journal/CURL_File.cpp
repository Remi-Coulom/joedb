#include "joedb/journal/CURL_File.h"
#include "joedb/journal/Sequential_File.h"
#include "joedb/error/Exception.h"

#include <cstring>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void CURL_File::error_check(CURLcode code)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (code != CURLE_OK)
   throw Exception(std::string("CURL_File: ") + curl_easy_strerror(code));
 }

 ////////////////////////////////////////////////////////////////////////////
 void CURL_File::perform_range(int64_t start, int64_t size) const
 ////////////////////////////////////////////////////////////////////////////
 {
  const std::string range =
   std::to_string(start) + '-' + std::to_string(start + size - 1);

  error_check(curl_easy_setopt(curl, CURLOPT_RANGE, range.c_str()));
  error_check(curl_easy_perform(curl));

  long code = 0;
  error_check(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code));

  if (code != 0 && code != 206)
   throw Exception("CURL_FIle: unexpected response code: " + std::to_string(code));
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t CURL_File::pread_Callback_Data::copy
 ////////////////////////////////////////////////////////////////////////////
 (
  char *contents,
  size_t real_size
 )
 {
  const size_t copy_size = std::min(size - offset, real_size);
  std::memcpy(buffer + offset, contents, copy_size);
  offset += copy_size;
  return copy_size;
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t CURL_File::pread_callback
 ////////////////////////////////////////////////////////////////////////////
 (
  void *contents,
  size_t size,
  size_t nmemb,
  void *p
 )
 {
  const size_t real_size = size * nmemb;
  pread_Callback_Data &callback_data = *(pread_Callback_Data *)p;
  return callback_data.copy(reinterpret_cast<char *>(contents), real_size);
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t CURL_File::pread(char *buffer, size_t size, int64_t offset) const
 ////////////////////////////////////////////////////////////////////////////
 {
  pread_Callback_Data callback_data{buffer, size, 0};

  error_check(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &callback_data));
  error_check(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, pread_callback));

  perform_range(offset, int64_t(size));

  return callback_data.offset;
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t CURL_File::copy_callback
 ////////////////////////////////////////////////////////////////////////////
 (
  void * const contents,
  const size_t size,
  const size_t nmemb,
  void * const p
 )
 {
  const size_t real_size = size * nmemb;
  Sequential_File &cursor = *((Sequential_File *)p);
  cursor.sequential_write((const char *)contents, real_size);
  return real_size;
 }

 ////////////////////////////////////////////////////////////////////////////
 void CURL_File::copy_to
 ////////////////////////////////////////////////////////////////////////////
 (
  Abstract_File &destination,
  const int64_t start,
  const int64_t size
 ) const
 {
  Sequential_File cursor(destination);
  error_check(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cursor));
  error_check(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, copy_callback));
  perform_range(start, size);
 }

 ////////////////////////////////////////////////////////////////////////////
 CURL_Easy::CURL_Easy(): curl(curl_easy_init())
 ////////////////////////////////////////////////////////////////////////////
 {
  if (curl == nullptr)
   throw Exception("Could not initialize CURL");
 }

 ////////////////////////////////////////////////////////////////////////////
 CURL_Easy::~CURL_Easy()
 ////////////////////////////////////////////////////////////////////////////
 {
  curl_easy_cleanup(curl);
 }

 ////////////////////////////////////////////////////////////////////////////
 CURL_File::CURL_File(const char *url, bool verbose):
 ////////////////////////////////////////////////////////////////////////////
  Abstract_File(Open_Mode::read_existing)
 {
  error_check(curl_easy_setopt(curl, CURLOPT_VERBOSE, verbose));
  error_check(curl_easy_setopt(curl, CURLOPT_URL, url));
  error_check(curl_easy_setopt(curl, CURLOPT_USERAGENT, "joedb"));
  error_check(curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L));
 }
}
