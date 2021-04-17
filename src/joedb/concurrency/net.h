#ifndef joedb_net_declared
#define joedb_net_declared

#if JOEDB_HAS_NET
#include <net>
namespace net = std::net;
#elif JOEDB_HAS_EXPERIMENTAL_NET
#include <experimental/net>
namespace net = std::experimental::net;
#elif JOEDB_HAS_BOOST_NET
#include <boost/asio/ts/net.hpp>
namespace net = boost::asio;
#else
#error No networking
#endif

#endif
