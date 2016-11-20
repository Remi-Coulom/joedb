#include "c_wrapper.h"
#include "Compiler_Options.h"

#include <iostream>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
void joedb::generate_c_wrapper
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &header,
 std::ostream &body,
 const joedb::Compiler_Options &options
)
{
 const std::string &name = options.get_namespace_name();

 ////////////////////////////////////////////////////////////////////////////
 // Header
 ////////////////////////////////////////////////////////////////////////////
 header << "#ifndef " << name << "_wrapper_declared\n";
 header << "#define " << name << "_wrapper_declared\n";

 header << "\n#ifdef __cplusplus\n";
 header << "extern \"C\" {\n";
 header << "#else\n";
 header << "#include <stdbool.h>\n";
 header << "#endif\n\n";
 
 header << "typedef struct " << name << "_db " << name << "_db;\n\n";

 header << name << "_db *" << name << "_open_file(const char *file_name, bool read_only);\n";
 header << "void " << name << "_delete(" << name << "_db *db);\n";
 header << "bool " << name << "_is_good(" << name << "_db *db);\n";

 header << "\n#ifdef __cplusplus\n";
 header << "}\n";
 header << "#endif\n\n";

 header << "#endif\n";

 ////////////////////////////////////////////////////////////////////////////
 // Body
 ////////////////////////////////////////////////////////////////////////////
 std::ostringstream convert;
 convert << " " << name << "::File_Database *p = (" << name;
 convert << "::File_Database *)db;\n";

 body << "#include \"" << name << "_wrapper.h\"\n";
 body << "#include \"" << name << ".h\"\n";
 body << '\n';

 body << name << "_db *" << name << "_open_file(const char *file_name, bool read_only)\n";
 body << "{\n";
 body << " return (" << name << "_db *)(new " << name << "::File_Database(file_name, read_only));\n";
 body << "}\n";

 body << "\nvoid " << name << "_delete(" << name << "_db *db)\n";
 body << "{\n";
 body << convert.str();
 body << " delete p;\n";
 body << "}\n";

 body << "\nbool " << name << "_is_good(" << name << "_db *db)\n";
 body << "{\n";
 body << convert.str();
 body << " return p->is_good();\n";
 body << "}\n";
}
