#ifndef joedb_generator_Generator_declared
#define joedb_generator_Generator_declared

#include "joedb/compiler/Compiler_Options.h"

#include <fstream>

namespace joedb::generator
{
 /// \ingroup compiler
 class Generator
 {
  protected:
   const Compiler_Options &options;
   std::ofstream out;

   bool db_has_values() const;

   void write_initial_comment();
   void write_type(Type type, bool return_type, bool setter_type);
   void write_tuple_type(const Compiler_Options::Index &index);
   void write_index_type(const Compiler_Options::Index &index);

   static const char *get_type_string(Type type);
   static const char *get_cpp_type_string(Type type);
   static const char *get_storage_type_string(Type type);

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
