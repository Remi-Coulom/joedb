#ifndef joedb_net_declared
#define joedb_net_declared

#if JOEDB_HAS_NET
#include <net>
namespace net = std::net;
#elif JOEDB_HAS_EXPERIMENTAL_NET
#include <experimental/net>
namespace net = std::experimental::net;
#elif JOEDB_HAS_BOOST_NET
#ifdef _WIN32
#include <windows.h>
#endif
#include <boost/asio/ts/net.hpp>
namespace net = boost::asio;
#elif JOEDB_HAS_ASIO_NET
#include <asio/ts/net.hpp>
namespace net = asio;
#else
#error No networking
#endif

#endif
