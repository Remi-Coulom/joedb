#include "joedb/ui/Progress_Bar.h"

#include <iostream>

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 Progress_Bar::Progress_Bar
 //////////////////////////////////////////////////////////////////////////
 (
  const int64_t total,
  std::ostream * const out,
  const int64_t threshold
 ):
  total(total),
  out(out),
  threshold(threshold),
  current_display(0)
 {
  if (out)
  {
   *out << ": size = " << total << '\n';
   if (total > threshold)
    *out << std::string(length, '.') << '\r';

   out->flush();
  }
 }

 //////////////////////////////////////////////////////////////////////////
 void Progress_Bar::print(const int64_t current)
 //////////////////////////////////////////////////////////////////////////
 {
  if (out && total > threshold)
  {
   const int display = int((current * length) / total);

   if (display > current_display)
   {
    *out << std::string(display - current_display, '#');
    out->flush();

    current_display = display;
   }
  }
 }

 //////////////////////////////////////////////////////////////////////////
 Progress_Bar::~Progress_Bar()
 //////////////////////////////////////////////////////////////////////////
 {
  try
  {
   if (out)
   {
    if (total > threshold)
     *out << '\n';
    out->flush();
   }
  }
  catch(...)
  {
  }
 }
}
