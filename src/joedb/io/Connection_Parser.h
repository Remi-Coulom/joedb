#ifndef joedb_Connection_Parser_declared
#define joedb_Connection_Parser_declared

#include <vector>
#include <memory>

namespace joedb
{
 class Connection_Builder;
 class Connection;

 ////////////////////////////////////////////////////////////////////////////
 class Connection_Parser
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::vector<std::unique_ptr<Connection_Builder>> builders;

  public:
   Connection_Parser(bool local);
   void list_builders() const;
   Connection_Builder *get_builder(const char *name) const;
   static std::unique_ptr<Connection> build
   (
    Connection_Builder &builder,
    int argc,
    char **argv
   );
   std::unique_ptr<Connection> build(int argc, char **argv) const;
 };
}

#endif
