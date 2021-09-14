#ifndef joedb_Logger
#define joedb_Logger

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Logger
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual void write(const char *message) noexcept = 0;
   virtual ~Logger() = default;
 };
}

#endif
