#include "joedb/ui/Progress_Bar.h"

#include <iostream>

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 Progress_Bar::Progress_Bar
 //////////////////////////////////////////////////////////////////////////
 (
  int64_t total,
  std::ostream *out
 ):
  total(total),
  out(out),
  current_display(0)
 {
  if (out)
  {
   if (total > threshold)
    *out << '\n' << std::string(length, '.') << '\r';
   else
    *out << ": size = " << total;

   out->flush();
  }
 }

 //////////////////////////////////////////////////////////////////////////
 void Progress_Bar::print(int64_t current)
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
    else
     *out << " done.\n";
    out->flush();
   }
  }
  catch(...)
  {
  }
 }
}
