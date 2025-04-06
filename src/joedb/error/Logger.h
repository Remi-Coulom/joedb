#ifndef joedb_Logger
#define joedb_Logger

namespace joedb
{
 /// @ingroup error
 class Logger
 {
  public:
   virtual void write(const char *message) noexcept = 0;
   virtual ~Logger() = default;
 };
}

#endif
