#ifndef joedb_generator_Generator_declared
#define joedb_generator_Generator_declared

#include "joedb/compiler/Compiler_Options.h"

#include <fstream>

namespace joedb::generator
{
 class Generator
 {
  protected:
   const Compiler_Options &options;
   std::ofstream out;

   bool db_has_values() const;

   void write_initial_comment();
   void write_type(Type type, bool return_type, bool setter_type);
   const char *get_type_string(Type type);
   const char *get_cpp_type_string(Type type);
   const char *get_storage_type_string(Type type);

  public:
   Generator
   (
    const char *dir_name,
    const char *file_name,
    const Compiler_Options &options
   );

   virtual void generate() = 0;

   virtual ~Generator();
 };
}

#endif
