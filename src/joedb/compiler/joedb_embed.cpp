#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "joedb/io/type_io.h"
#include "joedb/io/base64.h"
#include "joedb/compiler/nested_namespace.h"

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc != 4 && argc != 5)
 {
  std::cerr << "usage: " << argv[0];
  std::cerr << " [--base64|raw|escape] <file_name.joedb> <namespace> <identifier>\n";
  std::cerr << "output: <namespace>_<identifer>.{h,cpp}\n";
  return 1;
 }

 enum Mode {base64, raw, escape};

 Mode mode = escape;

 if (argc == 5)
 {
  argv++;
  if (argv[0] == std::string("--base64"))
   mode = base64;
  else if (argv[0] == std::string("--raw"))
   mode = raw;
  else if (argv[0] == std::string("--escape"))
   mode = escape;
 }

 char const * const joedb_file_name = argv[1];
 std::vector<std::string> name_space = joedb::split_namespace(argv[2]);
 char const * const identifier = argv[3];

 std::ostringstream file_name;
 file_name << name_space.back() << '_' << identifier;

 {
  std::ofstream cpp
  (
   file_name.str() + ".cpp",
   std::ios::binary | std::ios::out
  );

  std::ostringstream file_content;

  {
   std::ifstream in(joedb_file_name, std::ios::binary | std::ios::in);
   if (!in.good())
   {
    std::cerr << "Error opening " << joedb_file_name << '\n';
    return 1;
   }
   file_content << in.rdbuf();
  }

  cpp << "#include \"" << file_name.str() << ".h\"\n";
  cpp << "#include \"" << name_space.back() << "_readonly.h\"\n";
  cpp << "#include \"joedb/journal/Readonly_Memory_File.h\"\n";
  if (mode == base64)
   cpp << "#include \"joedb/io/base64.h\"\n";
  cpp << '\n';
  cpp << "#include <memory>\n\n";

  joedb::namespace_open(cpp, name_space);

  if (mode != base64)
  {
   cpp << " const size_t " << identifier << "_size = ";
   cpp << file_content.str().size() << ";\n";
  }
  cpp << " char const * const " << identifier << "_data = ";

  if (mode == raw)
  {
   char const * const delimiter = "glouglou";
   // TODO: generate delimiter from data
   cpp << "R\"" << delimiter << "(";
   cpp << file_content.str();
   cpp << ")" << delimiter << '\"';
  }
  else if (mode == escape)
   joedb::write_string(cpp, file_content.str());
  else if (mode == base64)
   cpp << '"' << joedb::base64_encode(file_content.str()) << '"';

  cpp << ";\n\n";

  cpp << " struct struct_for_" << identifier << '\n';
  cpp << " {\n";
  cpp << "  std::unique_ptr<Database> db;\n";
  cpp << '\n';
  cpp << "  struct_for_" << identifier << "()\n";
  cpp << "  {\n";

  if (mode == base64)
  {
   cpp << "   const std::string decoded(joedb::base64_decode(" << identifier << "_data));\n";
   cpp << "   joedb::Readonly_Memory_File file(&decoded[0], decoded.size());\n";
  }
  else
  {
   cpp << "   joedb::Readonly_Memory_File file(" << identifier << "_data, " << identifier << "_size);\n";
  }

  cpp << "   db.reset(new Readonly_Database(file));\n";
  cpp << "  }\n";
  cpp << " };\n";
  cpp << '\n';
  cpp << " const Database &get_embedded_" << identifier << "()\n";
  cpp << " {\n";
  cpp << "  static struct_for_" << identifier << " s;\n";
  cpp << "  return *s.db;\n";
  cpp << " }\n";

  if (mode != base64)
  {
   cpp << "\n size_t get_embedded_" << identifier;
   cpp << "_size() {return " << identifier << "_size;}\n";

   cpp << " char const *get_embedded_" << identifier;
   cpp << "_data() {return " << identifier << "_data;}\n";
  }

  joedb::namespace_close(cpp, name_space);
 }

 {
  std::ofstream h(file_name.str() + ".h", std::ios::binary | std::ios::out);

  joedb::namespace_include_guard(h, identifier, name_space);

  h << "\n#include <stddef.h>\n\n";

  joedb::namespace_open(h, name_space);

  h << " class Database;\n";
  h << " const Database &get_embedded_" << identifier << "();\n";

  if (mode != base64)
  {
   h << " size_t get_embedded_" << identifier << "_size();\n";
   h << " char const *get_embedded_" << identifier << "_data();\n";
  }

  joedb::namespace_close(h, name_space);

  h << "#endif\n";
 }

 return 0;
}
