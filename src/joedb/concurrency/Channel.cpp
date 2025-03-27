#include "joedb/concurrency/Channel.h"

namespace joedb::concurrency
{
 void Channel::write(const char *data, size_t size)
 {
  size_t n = 0;
  while (n < size)
   n += write_some(data + n, size - n);
 }

 void Channel::read(char *data, size_t size)
 {
  size_t n = 0;
  while (n < size)
   n += read_some(data + n, size - n);
 }

 Channel::~Channel() = default;
}
