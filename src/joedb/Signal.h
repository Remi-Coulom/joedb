#ifndef joedb_Signal_declared
#define joedb_Signal_declared

#include <csignal>

#ifndef CDECL
#define CDECL
#endif

// Note: SIGUSR1 and SIGUSR2 are not available in Windows

#ifndef SIGUSR1
#define SIGUSR1 10
#endif

#ifndef SIGUSR2
#define SIGUSR2 12
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
