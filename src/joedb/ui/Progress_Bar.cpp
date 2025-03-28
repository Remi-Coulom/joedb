#include "joedb/ui/Progress_Bar.h"

#include <iostream>

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 Progress_Bar::Progress_Bar
 //////////////////////////////////////////////////////////////////////////
 (
  int64_t total,
  std::ostream &out
 ):
  total(total),
  out(out),
  current_display(0)
 {
  out << '\n';
  for (int i = length; --i >= 0;)
   out << '.';
  out << '\r';
  out.flush();
 }

 //////////////////////////////////////////////////////////////////////////
 void Progress_Bar::print(int64_t current)
 //////////////////////////////////////////////////////////////////////////
 {
  const int display = int((current * length) / total);

  if (display > current_display)
  {
   for (int i = display - current_display; --i >= 0;)
    out << '#';
   out.flush();

   current_display = display;
  }
 }

 //////////////////////////////////////////////////////////////////////////
 Progress_Bar::~Progress_Bar()
 //////////////////////////////////////////////////////////////////////////
 {
  try
  {
   out << '\n';
   out.flush();
  }
  catch(...)
  {
  }
 }
}
