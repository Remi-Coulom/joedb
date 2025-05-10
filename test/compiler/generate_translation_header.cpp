#include "db/test/Readonly_Database.h"
#include "joedb/ui/main_wrapper.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
static int generate_translation_header(joedb::Arguments &arguments)
/////////////////////////////////////////////////////////////////////////////
{
 my_namespace::is_nested::test::Readonly_Database db("test.joedb");

 std::cout << "#ifndef translation_declared\n";
 std::cout << "#define translation_declared\n\n";

 std::cout << "#include \"db/test/Database.h\"\n\n";

 std::cout << "namespace my_namespace\n";
 std::cout << "{\n";
 std::cout << " namespace is_nested\n";
 std::cout << " {\n";
 std::cout << "  namespace test\n";
 std::cout << "  {\n";

 std::cout << "   namespace string_id\n";
 std::cout << "   {\n";

 for (const auto string_id: db.get_string_id_table())
 {
  std::cout << "    constexpr id_of_string_id " << db.get_name(string_id);
  std::cout << '{' << string_id.get_id() << "};\n";
 }

 std::cout << "   }\n";

 std::cout << "   namespace language\n";
 std::cout << "   {\n";

 for (const auto language: db.get_language_table())
 {
  std::cout << "    constexpr id_of_language " << db.get_id(language);
  std::cout << '{' << language.get_id() << "};\n";
 }

 std::cout << "   }\n";
 std::cout << "  }\n";
 std::cout << " }\n";
 std::cout << "}\n\n";

 std::cout << "#endif\n";

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(generate_translation_header, argc, argv);
}
