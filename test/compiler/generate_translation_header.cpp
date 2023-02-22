#include "db/test_readonly.h"
#include "joedb/io/main_exception_catcher.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
static int generate_translation_header(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 my_namespace::is_nested::test::Readonly_Database db("test.joedb");

 std::cout << "#ifndef translation_declared\n";
 std::cout << "#define translation_declared\n\n";

 std::cout << "namespace translation\n";
 std::cout << "{\n";

 std::cout << " enum\n";
 std::cout << " {\n";

 for (auto string_id: db.get_string_id_table())
 {
  std::cout << "  " << db.get_name(string_id) << " = " << string_id.get_id();
  std::cout << ",\n";
 }

 std::cout << "  string_ids\n";
 std::cout << " };\n\n";

 std::cout << " namespace language\n";
 std::cout << " {\n";
 std::cout << "  enum\n";
 std::cout << "  {\n";

 for (auto language: db.get_language_table())
 {
  std::cout << "   " << db.get_id(language) << " = " << language.get_id();
  std::cout << ",\n";
 }

 std::cout << "   languages\n";
 std::cout << "  };\n";
 std::cout << " }\n";

 std::cout << "}\n\n";

 std::cout << "#endif\n";

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(generate_translation_header, argc, argv);
}
