#ifndef joedb_Signal_declared
#define joedb_Signal_declared

#include <csignal>
#include <atomic>

#ifndef CDECL
#define CDECL
#endif

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
  private:
   static void CDECL signal_handler(int sig);

  public:
   static constexpr int no_signal = 0;
   static std::atomic<int> signal;
   static void start();
   static void stop();
 };
}

#endif
