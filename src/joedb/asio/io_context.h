#ifndef joedb_asio_io_context_declared
#define joedb_asio_io_context_declared

#include <boost/asio/io_context.hpp>

namespace joedb::asio
{
 struct io_context
 {
  private:
   boost::asio::io_context value;

  public:
   io_context();

   boost::asio::io_context &operator*() {return value;}
   boost::asio::io_context *operator->() {return &value;}

   void run();

   ~io_context();
 };
}

#endif
