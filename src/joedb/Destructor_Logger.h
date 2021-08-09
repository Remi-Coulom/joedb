#ifndef joedb_Destructor_Logger
#define joedb_Destructor_Logger

namespace joedb
{
 class Logger;

 ////////////////////////////////////////////////////////////////////////////
 class Destructor_Logger
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   static Logger *the_logger;

  public:
   static void write(const char * message) noexcept;
   static void set_logger(Logger &new_logger);
 };
}

#endif
