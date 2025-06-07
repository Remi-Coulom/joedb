#include "joedb/compiler/generator/procedure_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 procedure_h::procedure_h
 (
  const Compiler_Options &options,
  const Compiler_Options &parent_options
 ):
  Generator
  (
   (parent_options.output_path + "/procedures").data(),
   (options.get_name_space_back() + ".h").data(),
   parent_options
  ),
  options(options)
 {
 }

 void procedure_h::generate()
 {
  auto name_space = Generator::options.name_space;
  const std::string &procedure_name = options.get_name_space_back();
  name_space.emplace_back("procedures");
  namespace_include_guard(out, procedure_name.data(), name_space);

  out << '\n';
  out << "#include \"" << procedure_name << "/Memory_Database.h\"\n";
  out << "#include \"../Client.h\"\n";
  out << '\n';

  namespace_open(out, name_space);

  out << "\n void execute\n";
  out << " (\n";
  out << "  Client &client,\n";
  out << "  " << procedure_name << "::Memory_Database &";
  out << procedure_name << '\n';
  out << " );\n";

  namespace_close(out, name_space);

  out << "\n#endif\n";
 }
}
