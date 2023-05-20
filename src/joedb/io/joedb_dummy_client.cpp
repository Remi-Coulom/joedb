#include "joedb/io/Connection_Builder.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Dummy_Connection_Builder: public Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   std::unique_ptr<Connection> build(int argc, char **argv) final
   {
    return std::unique_ptr<Connection>(new Connection());
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
