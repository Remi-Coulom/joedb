#include "joedb/journal/CURL_File.h"
#include <sstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void CURL_File::throw_if_error(CURLcode code)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (code != CURLE_OK)
   throw Exception(curl_easy_strerror(code));
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t CURL_File::Callback_Data::copy(char *contents, size_t real_size)
 ////////////////////////////////////////////////////////////////////////////
 {
  const size_t copy_size = std::min(size - offset, real_size);
  std::copy_n(contents, copy_size, buffer + offset);
  offset += copy_size;
  return copy_size;
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t CURL_File::callback
 ////////////////////////////////////////////////////////////////////////////
 (
  void *contents,
  size_t size,
  size_t nmemb,
  void *p
 )
 {
  const size_t real_size = size * nmemb;
  Callback_Data &callback_data = *reinterpret_cast<Callback_Data *>(p);
  return callback_data.copy(reinterpret_cast<char *>(contents), real_size);
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t CURL_File::raw_pread(char *buffer, size_t size, int64_t offset)
 ////////////////////////////////////////////////////////////////////////////
 {
  Callback_Data callback_data{buffer, size, 0};
  std::ostringstream range;
  range << offset << '-' << offset + size - 1;
  throw_if_error(curl_easy_setopt(curl, CURLOPT_RANGE, range.str().c_str()));
  throw_if_error(curl_easy_setopt(curl, CURLOPT_WRITEDATA, &callback_data));
  throw_if_error(curl_easy_perform(curl));
  return callback_data.offset;
 }

 ////////////////////////////////////////////////////////////////////////////
 void CURL_File::raw_pwrite(const char *buffer, size_t size, int64_t offset)
 ////////////////////////////////////////////////////////////////////////////
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t CURL_File::raw_get_size() const
 ////////////////////////////////////////////////////////////////////////////
 {
  return -1;
 }

 ////////////////////////////////////////////////////////////////////////////
 CURL_File::CURL_File(const char *url, bool verbose):
 ////////////////////////////////////////////////////////////////////////////
  Generic_File(Open_Mode::read_existing),
  curl(curl_easy_init())
 {
  if (curl == nullptr)
   throw Exception("Could not initialize CURL");

  throw_if_error(curl_easy_setopt(curl, CURLOPT_VERBOSE, verbose));
  throw_if_error(curl_easy_setopt(curl, CURLOPT_URL, url));
  throw_if_error(curl_easy_setopt(curl, CURLOPT_USERAGENT, "joedb"));
  throw_if_error(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback));
 }

 ////////////////////////////////////////////////////////////////////////////
 CURL_File::~CURL_File()
 ////////////////////////////////////////////////////////////////////////////
 {
  curl_easy_cleanup(curl);
 }
}
