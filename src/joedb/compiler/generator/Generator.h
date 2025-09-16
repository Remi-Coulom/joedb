#ifndef joedb_generator_Generator_declared
#define joedb_generator_Generator_declared

#include "joedb/compiler/Compiler_Options.h"

#include <ostream>

namespace joedb::generator
{
 /// @ingroup compiler
 class Generator
 {
  protected:
   const std::string dir_name;
   const std::string file_name;
   const Compiler_Options &options;

   bool db_has_values() const;

   void write_initial_comment(std::ostream &out);
   void write_type(std::ostream &out, Type type, bool return_type, bool setter_type);
   void write_tuple_type(std::ostream &out, const Compiler_Options::Index &index, bool reference);
   void write_index_type(std::ostream &out, const Compiler_Options::Index &index);

   static const char *get_type_string(Type type);
   static const char *get_cpp_type_string(Type type);
   static const char *get_storage_type_string(Type type);

  public:
   Generator
   (
    std::string dir_name,
    std::string file_name,
    const Compiler_Options &options
   );

   virtual void write(std::ostream &out) = 0;
   void generate();

   virtual ~Generator();
 };
}

#endif
