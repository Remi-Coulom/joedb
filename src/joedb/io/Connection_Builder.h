#ifndef joedb_Connection_Builder_declared
#define joedb_Connection_Builder_declared

#include <memory>

namespace joedb
{
 class Generic_File;
 class Connection;

 ////////////////////////////////////////////////////////////////////////////
 class Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   std::unique_ptr<Generic_File> local_file;
   void open_local_file(const char *file_name);

  public:
   virtual int get_min_parameters() const = 0;
   virtual int get_max_parameters() const = 0;
   virtual const char *get_parameters_description() const = 0;
   virtual void build(int argc, const char * const *argv) = 0;
   virtual Connection &get_connection() = 0;
   virtual Generic_File &get_file() = 0;
   virtual ~Connection_Builder() = default;

   int main(int argc, char **argv);
 };
}

#endif
