#include "joedb/io/print_date.h"

#include <ctime>
#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void print_date(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  const std::time_t t = std::time(nullptr);
  const std::tm *tm = std::localtime(&t);
  constexpr size_t buffer_size = 32;
  char buffer[buffer_size];
  // note: msvcrt.dll does not support %e (instead of %d) in the format
  std::strftime(buffer, buffer_size, "%b %d %Y %H:%M:%S", tm);
  out << buffer << '\n';
 }
}
