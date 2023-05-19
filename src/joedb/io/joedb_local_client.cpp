#include "joedb/io/Connection_Builder.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/concurrency/Local_Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Local_Connection_Builder: public Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   bool get_default_sharing() const final
   {
    return true;
   }

   std::unique_ptr<Connection> build
   (
    Writable_Journal &client_journal,
    int argc,
    const char * const *argv
   ) final
   {
    return std::unique_ptr<Connection>(new Local_Connection(client_journal));
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  return Local_Connection_Builder().main(argc, argv);
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
