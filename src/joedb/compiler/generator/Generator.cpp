#include "joedb/compiler/generator/Generator.h"
#include "joedb/get_version.h"
#include "joedb/ui/get_time_string.h"

#include <iostream>
#include <filesystem>

namespace joedb::compiler::generator
{
 ////////////////////////////////////////////////////////////////////////////
 bool Generator::db_has_values() const
 ////////////////////////////////////////////////////////////////////////////
 {
  for (const auto &[tid, tname]: options.db.get_tables())
  {
   if (options.db.get_freedom(tid).size() > 0)
    return true;
  }
  return false;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generator::write_initial_comment()
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "/////////////////////////////////////////////////////////////////////////////\n";
  out << "//\n";
  out << "// This code was automatically generated by the joedb compiler\n";
  out << "// https://www.remi-coulom.fr/joedb/\n";
  out << "//\n";
  out << "// Path to compiler: " << options.exe_path << '\n';
  out << "// Version: " << joedb::get_version() << '\n';
  out << "// joedbc compilation time: " << __DATE__ << ' ' << __TIME__ << '\n';
  out << "// Generation of this file: " << ui::get_time_string_of_now() << '\n';
  out << "//\n";
  out << "/////////////////////////////////////////////////////////////////////////////\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generator::write_type
 ////////////////////////////////////////////////////////////////////////////
 (
  Type type,
  bool return_type,
  bool setter_type
 )
 {
  switch (type.get_type_id())
  {
   case Type::Type_Id::null:
    out << "void";
   break;

   case Type::Type_Id::reference:
    out << "id_of_" << options.db.get_table_name(type.get_table_id());
   break;

   #define TYPE_MACRO(storage_tt, return_tt, type_id, read, write)\
   case Type::Type_Id::type_id:\
    if (return_type || setter_type)\
     out << #return_tt;\
    else\
     out << #storage_tt;\
   break;
   #define TYPE_MACRO_NO_REFERENCE
   #include "joedb/TYPE_MACRO.h"
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generator::write_tuple_type(const Compiler_Options::Index &index)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (index.field_ids.size() > 1)
  {
   out << "std::tuple<";

   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    if (i > 0)
     out << ", ";

    const Type &type = options.db.get_field_type
    (
     index.table_id,
     index.field_ids[i]
    );

    write_type(type, false, false);
   }

   out << ">";
  }
  else
  {
   write_type
   (
    options.db.get_field_type(index.table_id, index.field_ids[0]),
    false,
    false
   );
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generator::write_index_type(const Compiler_Options::Index &index)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "std::";
  if (index.unique)
   out << "map";
  else
   out << "multimap";
  out << '<';

  write_tuple_type(index);

  out << ", id_of_" << options.db.get_table_name(index.table_id) << ">";
 }

 ////////////////////////////////////////////////////////////////////////////
 // type arrays
 ////////////////////////////////////////////////////////////////////////////
 #define STRINGIFY(X) #X
 #define EXPAND_AND_STRINGIFY(X) STRINGIFY(X)

 ////////////////////////////////////////////////////////////////////////////
 const char *Generator::get_type_string(Type type)
 ////////////////////////////////////////////////////////////////////////////
 {
  static constexpr char const * const types[] =
  {
   nullptr,
  #define TYPE_MACRO(a, b, type_id, d, e) EXPAND_AND_STRINGIFY(type_id),
  #include "joedb/TYPE_MACRO.h"
  };

  return types[int(type.get_type_id())];
 }

 ////////////////////////////////////////////////////////////////////////////
 const char *Generator::get_cpp_type_string(Type type)
 ////////////////////////////////////////////////////////////////////////////
 {
  static char const * const cpp_types[] =
  {
   nullptr,
  #define TYPE_MACRO(a, type, c, d, e) EXPAND_AND_STRINGIFY(type),
  #include "joedb/TYPE_MACRO.h"
  };

  return cpp_types[int(type.get_type_id())];
 }

 ////////////////////////////////////////////////////////////////////////////
 const char *Generator::get_storage_type_string(Type type)
 ////////////////////////////////////////////////////////////////////////////
 {
  static char const * const storage_types[] =
  {
   nullptr,
  #define TYPE_MACRO(storage, b, c, d, e) EXPAND_AND_STRINGIFY(storage),
  #include "joedb/TYPE_MACRO.h"
  };

  return storage_types[int(type.get_type_id())];
 }

 #undef EXPAND_AND_STRINGIFY
 #undef STRINGIFY

 ////////////////////////////////////////////////////////////////////////////
 Generator::Generator
 ////////////////////////////////////////////////////////////////////////////
 (
  const char *dir_name,
  const char *file_name,
  const Compiler_Options &options
 ):
  options(options)
 {
  std::filesystem::create_directory(options.get_name_space().back());
  std::string dir_string = options.get_name_space().back() + "/" + std::string(dir_name);
  std::filesystem::create_directory(dir_string);
  std::string file_string = dir_string + "/" + std::string(file_name);

  out.exceptions(std::ios::badbit | std::ios::failbit);
  out.open(file_string, std::ios::trunc);
  write_initial_comment();
 }

 ////////////////////////////////////////////////////////////////////////////
 Generator::~Generator() = default;
 ////////////////////////////////////////////////////////////////////////////
}
