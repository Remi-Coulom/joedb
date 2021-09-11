#include "joedb/Destructor_Logger.h"
#include "joedb/Stream_Logger.h"

namespace joedb
{
 static Stream_Logger default_logger(std::cerr);
 Logger *Destructor_Logger::the_logger = &default_logger;

 ////////////////////////////////////////////////////////////////////////////
 void Destructor_Logger::write(const char * message) noexcept
 ////////////////////////////////////////////////////////////////////////////
 {
  if (the_logger)
   the_logger->write(message);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Destructor_Logger::set_logger(Logger *new_logger)
 ////////////////////////////////////////////////////////////////////////////
 {
  the_logger = new_logger;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Destructor_Logger::set_logger()
 ////////////////////////////////////////////////////////////////////////////
 {
  the_logger = &default_logger;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Destructor_Logger::remove_logger()
 ////////////////////////////////////////////////////////////////////////////
 {
  the_logger = nullptr;
 }
}
