#ifndef joedb_Parsed_Logger_declared
#define joedb_Parsed_Logger_declared

#include "joedb/error/Logger.h"
#include "joedb/ui/Arguments.h"

#include <memory>
#include <vector>

namespace joedb
{
 class Parsed_Logger
 {
  public:
   enum class Type
   {
    none,
    clog,
    system
   };

  private:
   const Type type;
   const int log_level;
   const std::string tag;

   std::unique_ptr<Logger> logger;

   static const std::vector<const char *> type_string;

  public:
   Parsed_Logger(Arguments &args, Type default_log_type = Type::clog);

   Logger &get() const {return *logger;}
   int get_log_level() const {return log_level;}
   const std::string &get_tag() const {return tag;}
 };
}

#endif
