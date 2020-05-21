#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc != 4)
 {
  std::cerr << "usage: " << argv[0];
  std::cerr << " <file_name.joedb> <namespace> <identifier>\n";
  std::cerr << "output: <namespace>_<identifer>.{h,cpp}\n";
  return 1;
 }

 char const * const joedb_file_name = argv[1];
 char const * const namespace_name = argv[2];
 char const * const identifier = argv[3];

 std::ostringstream file_name;
 file_name << namespace_name << '_' << identifier;

 {
  std::ofstream cpp(file_name.str() + ".cpp");

  std::ostringstream file_content;
  file_content << std::ifstream(joedb_file_name).rdbuf();

  char const * const delimiter = "glouglou";
  // TODO: generate delimiter from data

  cpp << "#include \"" << file_name.str() << ".h\"\n";
  cpp << "#include \"" << namespace_name << ".h\"\n";
  cpp << '\n';
  cpp << "#include <sstream>\n";
  cpp << '\n';

  cpp << "namespace " << namespace_name << '\n';
  cpp << "{\n";

  cpp << " const size_t " << identifier << "_size = ";
  cpp << file_content.str().size() << ";\n";
  cpp << " char const * const " << identifier << "_data = R\"";
  cpp << delimiter << "(";
  cpp << file_content.str();
  cpp << ")" << delimiter << "\";\n";

  cpp << '\n';
  cpp << " const Database &get_embedded_" << identifier << "()\n";
  cpp << " {\n";
  cpp << "  static std::istringstream iss(std::string(" << identifier;
  cpp << "_data, " << identifier << "_size));\n";
  cpp << "  static joedb::Input_Stream_File file(iss);\n";
  cpp << "  static Generic_Readonly_Database db(file);\n";
  cpp << "  return db;\n";
  cpp << " }\n";
  // TODO: use a stream that does not copy the string

  cpp << "}\n";
 }

 {
  std::ofstream h(file_name.str() + ".h");

  {
   std::ostringstream guard_macro;
   guard_macro << namespace_name << '_' << identifier << "_declared";

   h << "#ifndef " << guard_macro.str() << '\n';
   h << "#define " << guard_macro.str() << '\n';
   h << '\n';
  }

  h << "namespace " << namespace_name << '\n';
  h << "{\n";
  h << " class Database;\n";
  h << " const Database &get_embedded_" << identifier << "();\n";
  h << "}\n";
  h << '\n';

  h << "#endif\n";
 }

 return 0;
}
