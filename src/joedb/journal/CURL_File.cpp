#include "joedb/journal/CURL_File.h"
#include <sstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void CURL_File::error_check(CURLcode code)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (code != CURLE_OK)
   throw Exception(curl_easy_strerror(code));
 }

 ////////////////////////////////////////////////////////////////////////////
 void CURL_File::perform_range(int64_t start, int64_t size)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::ostringstream range;
  range << start << '-' << start + size - 1;

  error_check(curl_easy_setopt(curl, CURLOPT_RANGE, range.str().c_str()));
  error_check(curl_easy_perform(curl));

  long code = 0;
  error_check(curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code));

  if (code != 0 && code != 206)
  {
   std::ostringstream error_message;
   error_message << "unexpected response code: " << code;
   throw Exception(error_message.str());
  }
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
  std::copy_n(contents, copy_size, buffer + offset);
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
 size_t CURL_File::pread(char *buffer, size_t size, int64_t offset)
 ////////////////////////////////////////////////////////////////////////////
 {
  pread_Callback_Data callback_data{buffer, size, 0};

  error_check(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &callback_data));
  error_check(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, pread_callback));

  perform_range(offset, size);

  return callback_data.offset;
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t CURL_File::copy_callback
 ////////////////////////////////////////////////////////////////////////////
 (
  void *contents,
  size_t size,
  size_t nmemb,
  void *p
 )
 {
  const size_t real_size = size * nmemb;
  Generic_File &destination = *((Generic_File *)p);
  destination.pos_write((const char *)contents, real_size);
  return real_size;
 }

 ////////////////////////////////////////////////////////////////////////////
 void CURL_File::copy_to
 ////////////////////////////////////////////////////////////////////////////
 (
  Generic_File &destination,
  int64_t start,
  int64_t size
 )
 {
  destination.set_position(start);

  error_check(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &destination));
  error_check(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, copy_callback));

  perform_range(start, size);
 }

 ////////////////////////////////////////////////////////////////////////////
 void CURL_File::pwrite(const char *buffer, size_t size, int64_t offset)
 ////////////////////////////////////////////////////////////////////////////
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t CURL_File::get_size() const
 ////////////////////////////////////////////////////////////////////////////
 {
  return -1;
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
  Generic_File(Open_Mode::read_existing)
 {
  error_check(curl_easy_setopt(curl, CURLOPT_VERBOSE, verbose));
  error_check(curl_easy_setopt(curl, CURLOPT_URL, url));
  error_check(curl_easy_setopt(curl, CURLOPT_USERAGENT, "joedb"));
  error_check(curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L));
 }
}
