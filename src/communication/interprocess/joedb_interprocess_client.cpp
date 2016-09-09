#include "joedb_interprocess.h"

#include <boost/interprocess/ipc/message_queue.hpp>
#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc < 2)
 {
  std::cerr << "usage: " << argv[0] << " <server_name>\n";
  return 1;
 }
 const char *name = argv[1];

 try
 {
  boost::interprocess::message_queue mq(boost::interprocess::open_only, name);
  message_code_t message = 123;
  mq.send(&message, sizeof(message), 0);
 }
 catch (boost::interprocess::interprocess_exception &e)
 {
  std::cerr << "boost::interprocess: " << e.what() << '\n';
  return 1;
 }

 return 0;
}
