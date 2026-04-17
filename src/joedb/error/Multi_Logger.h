#ifndef joedb_Multi_Logger_declared
#define joedb_Multi_Logger_declared

#include "joedb/error/Logger.h"

#include <vector>
#include <memory>

namespace joedb
{
 class Multi_Logger: public Logger
 {
  private:
   std::vector<std::unique_ptr<Logger>> loggers;

  public:
   void log(beman::cstring_view message) noexcept override
   {
    for (const auto &logger: loggers)
     logger->log(message);
   }

   void clear() 
   {
    loggers.clear();
   }

   void add(std::unique_ptr<Logger> logger)
   {
    loggers.emplace_back(std::move(logger));
   }
 };
}

#endif
