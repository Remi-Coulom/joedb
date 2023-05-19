#ifndef joedb_Connection_Builder_declared
#define joedb_Connection_Builder_declared

#include <memory>

namespace joedb
{
 class Connection;
 class Writable_Journal;

 ////////////////////////////////////////////////////////////////////////////
 class Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual int get_min_parameters() const {return 0;}
   virtual int get_max_parameters() const {return 0;}
   virtual const char *get_parameters_description() const {return "";}
   virtual std::unique_ptr<Connection> build
   (
    Writable_Journal &client_journal,
    int argc,
    const char * const *argv
   ) = 0;

   virtual ~Connection_Builder() = default;

   int main(int argc, char **argv);
 };
}

#endif
