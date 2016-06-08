#include <iostream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/array.hpp>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc < 3)
 {
  std::cerr << "usage: " << argv[0] << " <host> <port>\n";
  return 1;
 }
 else try
 {
  std::cout << "Connecting to " << argv[1] << " on port " << argv[2] << '\n';

  boost::asio::io_service aios;
  boost::asio::ip::tcp::resolver resolver(aios);
  boost::asio::ip::tcp::resolver::iterator endpoint =
   resolver.resolve(boost::asio::ip::tcp::resolver::query(argv[1], argv[2]));
  boost::asio::ip::tcp::socket socket(aios);
  boost::asio::connect(socket, endpoint);

  while (true)
  {
   boost::array<char, 4> buf;
   boost::system::error_code error;
   size_t len = socket.read_some(boost::asio::buffer(buf), error);

   if (error == boost::asio::error::eof)
    break;
   else if (error)
    throw boost::system::system_error(error);

   std::cout.write(buf.data(), long(len));
  }

  std::cout << '\n';
 }
 catch (const std::exception &e)
 {
  std::cerr << "exception: " << e.what() << '\n';
  return 1;
 }

 return 0;
}
