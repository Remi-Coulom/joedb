#ifndef joedb_error_Destructor_Logger
#define joedb_error_Destructor_Logger

namespace joedb::error
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
   static void set_logger(Logger *new_logger);
   static void remove_logger();
   static void set_logger();
 };
}

#endif
