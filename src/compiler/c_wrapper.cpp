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
 const Database &db = options.get_db();
 auto tables = db.get_tables();

 ////////////////////////////////////////////////////////////////////////////
 // Header
 ////////////////////////////////////////////////////////////////////////////
 header << "#ifndef " << name << "_wrapper_declared\n";
 header << "#define " << name << "_wrapper_declared\n";

 header << "#include <stddef.h>\n";
 header << "#include \"index_types.h\"\n";

 header << "\n#ifdef __cplusplus\n";
 header << "extern \"C\" {\n";
 header << "#else\n";
 header << "#include <stdbool.h>\n";
 header << "#endif\n\n";

 header << "typedef struct " << name << "_db " << name << "_db;\n";
 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  header << "typedef record_id_t " << name << "_id_of_" << tname << ";\n";
 }

 header << '\n';
 header << name << "_db *" << name << "_open_file(const char *file_name, bool read_only);\n";
 header << "void " << name << "_delete(" << name << "_db *db);\n";
 header << '\n';
 header << "bool " << name << "_is_good(" << name << "_db *db);\n";
 header << "void " << name << "_checkpoint_no_commit(" << name << "_db *db);\n";
 header << "void " << name << "_checkpoint_half_commit(" << name << "_db *db);\n";
 header << "void " << name << "_checkpoint_full_commit(" << name << "_db *db);\n";
 header << '\n';
 header << "void " << name << "_timestamp(" << name << "_db *db);\n";
 header << "void " << name << "_comment(" << name << "_db *db, const char *comment);\n";
 header << "void " << name << "_valid_data(" << name << "_db *db);\n";

 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  const auto storage = options.get_table_options(table.first).storage;
  const bool has_delete = storage == Compiler_Options::Table_Storage::freedom_keeper;

  header << '\n';
  header << name << "_id_of_" << tname << ' ';
  header << name << "_new_" << tname << '(' << name << "_db *db);\n";
  header << name << "_id_of_" << tname << ' ';
  header << name << "_new_vector_of_" << tname;
  header << '(' << name << "_db *db, size_t size);\n";

  if (has_delete)
  {
   header << "void " << name << "_delete_" << tname;
   header << '(' << name << "_db *db, ";
   header << name << "_id_of_" << tname << " id);\n";
  }
 }

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
 body << "}\n\n";

 body << "\nvoid " << name << "_checkpoint_no_commit(" << name << "_db *db)\n";
 body << "{\n";
 body << convert.str();
 body << " p->checkpoint_no_commit();\n";
 body << "}\n";

 body << "\nvoid " << name << "_checkpoint_half_commit(" << name << "_db *db)\n";
 body << "{\n";
 body << convert.str();
 body << " p->checkpoint_half_commit();\n";
 body << "}\n";

 body << "\nvoid " << name << "_checkpoint_full_commit(" << name << "_db *db)\n";
 body << "{\n";
 body << convert.str();
 body << " p->checkpoint_full_commit();\n";
 body << "}\n";

 body << "\nvoid " << name << "_timestamp(" << name << "_db *db)\n";
 body << "{\n";
 body << convert.str();
 body << " p->timestamp();\n";
 body << "}\n";
 body << "\nvoid " << name << "_comment(" << name << "_db *db, const char *comment)\n";
 body << "{\n";
 body << convert.str();
 body << " p->comment(comment);\n";
 body << "}\n";
 body << "\nvoid " << name << "_valid_data(" << name << "_db *db)\n";
 body << "{\n";
 body << convert.str();
 body << " p->valid_data();\n";
 body << "}\n";
 body << '\n';

 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  const auto storage = options.get_table_options(table.first).storage;
  const bool has_delete = storage == Compiler_Options::Table_Storage::freedom_keeper;

  body << name << "_id_of_" << tname << ' ';
  body << name << "_new_" << tname << '(' << name << "_db *db)\n{\n";
  body << convert.str();
  body << ' ' << "return p->new_" << tname << "().get_id();\n";
  body << "}\n\n";

  body << name << "_id_of_" << tname << ' ';
  body << name << "_new_vector_of_" << tname;
  body << '(' << name << "_db *db, size_t size)\n{\n";
  body << convert.str();
  body << ' ' << "return p->new_vector_of_" << tname << "(size).get_id();\n";
  body << "}\n\n";

  if (has_delete)
  {
   body << "void " << name << "_delete_" << tname;
   body << '(' << name << "_db *db, ";
   body << name << "_id_of_" << tname << " id)\n{\n";
   body << convert.str();
   body << ' ' << "p->delete_" << tname;
   body << '(' << name << "::id_of_" << tname << "(id));\n";
   body << "}\n\n";
  }
 }
}
