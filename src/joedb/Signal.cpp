#include "joedb/Signal.h"

extern "C"
{
 static sig_atomic_t signal_status;

 // Note: in C++11 signal handlers must have C linkage
 void CDECL joedb_signal_handler(int status)
 {
  signal_status = status;
 }
}

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Signal::set_signal(int status)
 ////////////////////////////////////////////////////////////////////////////
 {
  signal_status = status;
 }

 ////////////////////////////////////////////////////////////////////////////
 int Signal::get_signal()
 ////////////////////////////////////////////////////////////////////////////
 {
  return signal_status;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Signal::start()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::signal(SIGINT, joedb_signal_handler);
  std::signal(SIGUSR1, joedb_signal_handler);
  std::signal(SIGUSR2, joedb_signal_handler);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Signal::stop()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::signal(SIGINT, SIG_DFL);
  std::signal(SIGUSR1, SIG_DFL);
  std::signal(SIGUSR2, SIG_DFL);
 }
}
