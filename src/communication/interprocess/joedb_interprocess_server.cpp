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
 int current_client_id = 0;

 try
 {
  boost::interprocess::message_queue::remove(name);
  boost::interprocess::message_queue mq
  (
   boost::interprocess::create_only,
   name,
   1000,
   sizeof(message_code_t)
  );

  while (true)
  {
   message_code_t message;
   boost::interprocess::message_queue::size_type received_size;
   unsigned int priority;

   mq.receive(&message, sizeof(message), received_size, priority);
   std::cout << "message: " << message << '\n';
   std::cout.flush();
  }
 }
 catch (boost::interprocess::interprocess_exception &e)
 {
  std::cerr << e.what() << '\n';
  return 1;
 }

 return 0;
}
