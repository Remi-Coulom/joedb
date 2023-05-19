#include "joedb/io/Connection_Builder.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/concurrency/Dummy_Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Dummy_Connection_Builder: public Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   std::unique_ptr<Connection> build
   (
    Writable_Journal &client_journal,
    int argc,
    const char * const *argv
   ) final
   {
    return std::unique_ptr<Connection>(new Dummy_Connection(client_journal));
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  return Dummy_Connection_Builder().main(argc, argv);
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
