#include "joedb/ui/Parsed_Logger.h"
#include "joedb/error/CLog_Logger.h"
#include "joedb/error/System_Logger.h"

namespace joedb
{
 const std::vector<const char *> Parsed_Logger::type_string
 {
  "none",
  "clog",
  "system"
 };

 Parsed_Logger::Parsed_Logger(Arguments &args, Type default_log_type):
  type
  (
   Type
   (
    args.get_enum_option("log", type_string, size_t(default_log_type))
   )
  ),
  log_level(args.get_option<int>("log_level", "level", 100)),
  tag(args.get_string_option("log_tag", "tag", ""))
 {
  switch(type)
  {
   case Type::none:
    logger = std::make_unique<Logger>();
   break;

   case Type::clog:
    logger = std::make_unique<CLog_Logger>(tag);
   break;

   case Type::system:
    logger = std::make_unique<System_Logger>(tag);
   break;
  }
 }
}
