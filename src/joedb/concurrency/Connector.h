#ifndef joedb_Connector_declared
#define joedb_Connector_declared

#include "joedb/concurrency/Channel.h"

#include <memory>
#include <chrono>

namespace joedb
{
 /// Used by @ref Robust_Connection to reconnect after an error
 ///
 /// @ingroup concurrency
 class Connector
 {
  private:
   std::chrono::milliseconds keep_alive_interval = std::chrono::seconds(0);

  public:
   virtual std::unique_ptr<Channel> new_channel() const = 0;
   virtual ~Connector() = default;

   std::chrono::milliseconds get_keep_alive_interval() const
   {
    return keep_alive_interval;
   }

   Connector &set_keep_alive_interval(std::chrono::milliseconds interval)
   {
    keep_alive_interval = interval;
    return *this;
   }
 };
}

#endif
