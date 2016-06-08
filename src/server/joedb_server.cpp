#include <iostream>
#include <sstream>
#include <boost/asio.hpp>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc < 2)
 {
  std::cerr << "usage: " << argv[0] << " <port>\n";
  return 1;
 }
 else try
 {
  uint16_t port = 0;
  std::istringstream(argv[1]) >> port;
  std::cout << "Starting server on port " << port << "...\n";

  boost::asio::io_service aios;
  boost::asio::ip::tcp::acceptor acceptor(aios,
   boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

  std::cout << "Server ready" << std::endl;

  int message_number = 0;

  while (true)
  {
   message_number++;
   std::ostringstream oss;
   oss << "Message number: " << message_number;

   boost::asio::ip::tcp::socket socket(aios);
   acceptor.accept(socket);
   boost::asio::write(socket, boost::asio::buffer(oss.str()));
   std::cout << "Message sent: " << oss.str() << '\n';
  }
 }
 catch(const std::exception &e)
 {
  std::cerr << "exception: " << e.what() << '\n';
  return 1;
 }

 return 0;
}
