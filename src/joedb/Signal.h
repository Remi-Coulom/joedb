#ifndef joedb_Signal_declared
#define joedb_Signal_declared

#include <csignal>

#ifndef CDECL
#define CDECL
#endif

namespace joedb
{
 class Signal
 {
  public:
   static constexpr int no_signal = 0;
   static void set_signal(int status);
   static int get_signal();
   static void start();
   static void stop();
 };
}

#endif
