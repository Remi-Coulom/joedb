#include "joedb/io/Connection_Builder.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/concurrency/Dummy_Connection.h"
#include "joedb/journal/File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Dummy_Connection_Builder: public Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::unique_ptr<Dummy_Connection> connection;

  public:
   int get_min_parameters() const override {return 1;}
   int get_max_parameters() const override {return 1;}
   const char *get_parameters_description() const override
   {
    return "<file_name>";
   }

   void build(int argc, const char * const *argv) override
   {
    const char *file_name = argv[0];
    open_client_file(file_name);
    connection.reset(new Dummy_Connection(*client_journal));
   }

   Connection &get_connection() override {return *connection;}
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
