#include "joedb/ui/type_io.h"
#include "joedb/ui/base64.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/compiler/write_atomically.h"
#include "joedb/journal/File.h"

#include <iostream>
#include <string>

namespace joedb
{
 static int embed(Arguments &arguments)
 {
  enum class Mode {base64, raw, utf8, ascii};
  const std::vector<const char *> labels{"base64", "raw", "utf8", "ascii"};

  const Mode mode = Mode
  (
   arguments.get_enum_option("mode", labels, int(Mode::ascii))
  );
  const std::string_view joedb_file_name = arguments.get_next("<file.joedb>");
  const std::string_view namespace_string = arguments.get_next("<namespace>");
  const std::string_view identifier = arguments.get_next("<identifier>");

  const std::vector<std::string> name_space = split_namespace(namespace_string);

  if (arguments.missing() || name_space.empty())
  {
   arguments.print_help(std::cerr);
   std::cerr << "output: <namespace>_<identifer>.{h,cpp}\n";
   return 1;
  }

  const std::string file_name = name_space.back() + '_' + std::string(identifier);

  joedb::write_atomically(".", file_name + ".cpp", [&](std::ostream &cpp)
  {
   cpp << "#include \"" << file_name << ".h\"\n";
   cpp << "#include \"" << name_space.back() << "/Readonly_Database.h\"\n";
   cpp << "#include \"joedb/journal/Readonly_Memory_File.h\"\n";
   if (mode == Mode::base64)
    cpp << "#include \"joedb/ui/base64.h\"\n";
   cpp << '\n';

   namespace_open(cpp, name_space);
   cpp << '\n';

   {
    std::string file_content;

    {
     const File in(joedb_file_name.data(), Open_Mode::read_existing);
     file_content.resize(size_t(in.get_size()));
     in.pread(file_content.data(), file_content.size(), 0);
    }

    if (mode != Mode::base64)
    {
     cpp << " const size_t " << identifier << "_size = ";
     cpp << file_content.size() << ";\n";
    }
    cpp << " char const * const " << identifier << "_data = ";

    if (mode == Mode::base64)
    {
     cpp << '"' << base64_encode(file_content) << '"';
    }
    else if (mode == Mode::raw)
    {
     char const * const delimiter = "glouglou";
     // TODO: generate delimiter from data
     cpp << "R\"" << delimiter << "(";
     cpp << file_content;
     cpp << ")" << delimiter << '\"';
    }
    else if (mode == Mode::utf8)
    {
     write_string(cpp, file_content);
    }
    else if (mode == Mode::ascii)
    {
     cpp << '"';
     for (const uint8_t c: file_content)
      write_octal_character(cpp, c);
     cpp << '"';
    }
   }

   cpp << ";\n\n";

   cpp << " const Database &get_embedded_" << identifier << "()\n";
   cpp << " {\n";
   cpp << "  static const Readonly_Database db(joedb::Readonly_Memory_File(";

   if (mode == Mode::base64)
    cpp << "joedb::base64_decode(" << identifier << "_data)";
   else
    cpp << identifier << "_data, " << identifier << "_size";

   cpp << "));\n";
   cpp << "  return db;\n";
   cpp << " }\n";

   if (mode != Mode::base64)
   {
    cpp << "\n size_t get_embedded_" << identifier;
    cpp << "_size() {return " << identifier << "_size;}\n";

    cpp << " char const *get_embedded_" << identifier;
    cpp << "_data() {return " << identifier << "_data;}\n";
   }

   namespace_close(cpp, name_space);
  });

  joedb::write_atomically(".", file_name + ".h", [&](std::ostream &h)
  {
   namespace_include_guard_open(h, identifier.data(), name_space);

   h << "\n#include <stddef.h>\n\n";

   namespace_open(h, name_space);

   h << "\n class Database;\n";
   h << " const Database &get_embedded_" << identifier << "();\n";

   if (mode != Mode::base64)
   {
    h << " size_t get_embedded_" << identifier << "_size();\n";
    h << " char const *get_embedded_" << identifier << "_data();\n";
   }

   namespace_close(h, name_space);
   namespace_include_guard_close(h);
  });

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::embed, argc, argv);
}
