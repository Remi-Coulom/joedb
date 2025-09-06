#ifdef JOEDB_HAS_WEBSOCKETS
#include "joedb/concurrency/Websocket_Channel.h"

#include "gtest/gtest.h"

#include <array>

namespace joedb
{
 TEST(Websocket, basic)
 {
  GTEST_SKIP();
  Websocket_Channel channel("localhost", "8080", "/");
  std::string_view message("Hello");

  for (int count = 10; --count >= 0;)
  {
   channel.write(message.data(), message.size());
   std::array<char, 128> buffer;
   const size_t n = channel.read_some(buffer.data(), buffer.size());
   std::cerr << count << ' ' << std::string_view(buffer.data(), n) << '\n';
  }
 }
}
#endif
