#ifndef joedb_Connector_declared
#define joedb_Connector_declared

#include "joedb/concurrency/Channel.h"

#include <memory>

namespace joedb
{
 /// Used by @ref Robust_Connection to reconnect after an error
 ///
 /// @ingroup concurrency
 class Connector
 {
  public:
   virtual std::unique_ptr<Channel> new_channel() const = 0;
   virtual ~Connector() = default;
 };
}

#endif
