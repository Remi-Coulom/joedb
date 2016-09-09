#include <iostream>
#include <sstream>
#include <boost/asio.hpp>

#include "File.h"
#include "file_error_message.h"

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // Usage information if wrong number of arguments
 //
 if (argc < 3)
 {
  std::cerr << "usage: " << argv[0] << " <database> <port>\n";
  return 1;
 }
 else
 {
  //
  // Try to open the database file
  //
  std::cerr << "opening: " << argv[1] << "...\n";
  joedb::File file(argv[1], joedb::File::mode_t::write_existing);
  if (joedb::file_error_message(std::cerr, file))
   return 1;

  //
  // Server loop
  //
  try
  {
   uint16_t port = 0;
   std::istringstream(argv[2]) >> port;
   std::cout << "Starting server on port " << port << "...\n";

   boost::asio::io_service io_service;
   boost::asio::ip::tcp::acceptor acceptor(io_service,
    boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

   std::cout << "Server ready" << std::endl;

   int message_number = 0;

   while (true)
   {
    message_number++;
    std::ostringstream oss;
    oss << "Message number: " << message_number;

    boost::asio::ip::tcp::socket socket(io_service);
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
 }

 return 0;
}
