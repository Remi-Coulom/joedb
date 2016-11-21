#include "c_wrapper.h"
#include "Compiler_Options.h"

#include <iostream>
#include <sstream>

namespace joedb {

/////////////////////////////////////////////////////////////////////////////
void write_c_type
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Compiler_Options &options,
 Type type
)
{
 const Database &db = options.get_db();

 switch (type.get_type_id())
 {
  case Type::type_id_t::null:
   out << "void ";
  break;

  case Type::type_id_t::string:
   out << "char const *";
  break;

  case Type::type_id_t::reference:
  {
   const table_id_t referred = type.get_table_id();
   out << options.get_namespace_name() << "_id_of_";
   out << db.get_tables().find(referred)->second.get_name() << ' ';
  }
  break;

  #define TYPE_MACRO(type, return_type, type_id, read, write)\
  case Type::type_id_t::type_id:\
   out << #type << ' ';\
  break;
  #define TYPE_MACRO_NO_STRING
  #define TYPE_MACRO_NO_REFERENCE
  #include "TYPE_MACRO.h"
  #undef TYPE_MACRO_NO_REFERENCE
  #undef TYPE_MACRO_NO_STRING
  #undef TYPE_MACRO
 }
}

/////////////////////////////////////////////////////////////////////////////
void generate_c_wrapper
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &header,
 std::ostream &body,
 const Compiler_Options &options
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
  header << name << "_id_of_" << tname << ' ';
  header << name << "_get_beginning_of_" << tname << '(' << name << "_db *db);\n";
  header << name << "_id_of_" << tname << ' ';
  header << name << "_get_end_of_" << tname << '(' << name << "_db *db);\n";
  header << name << "_id_of_" << tname << ' ';
  header << name << "_get_next_" << tname << '(' << name << "_db *db, " << name << "_id_of_" << tname <<" id);\n";

  if (has_delete)
  {
   header << "void " << name << "_delete_" << tname;
   header << '(' << name << "_db *db, ";
   header << name << "_id_of_" << tname << " id);\n";
  }

  for (const auto &field: table.second.get_fields())
  {
   const std::string &fname = field.second.get_name();
   write_c_type(header, options, field.second.get_type());
   header << name << "_get_" << tname << '_' << fname << "(";
   header << name << "_db *db, ";
   header << name << "_id_of_" << tname;
   header << " id);\n";
   header << "void " << name << "_set_" << tname << '_' << fname << "(";
   header << name << "_db *db, ";
   header << name << "_id_of_" << tname << " id, ";
   write_c_type(header, options, field.second.get_type());
   header << "value);\n";
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

  body << name << "_id_of_" << tname << ' ';
  body << name << "_get_beginning_of_" << tname << '(' << name << "_db *db)\n{\n";
  body << convert.str();
  body << " return p->get_beginning_of_" << tname << "().get_id();\n";
  body << "}\n\n";

  body << name << "_id_of_" << tname << ' ';
  body << name << "_get_end_of_" << tname << '(' << name << "_db *db)\n{\n";
  body << convert.str();
  body << " return p->get_end_of_" << tname << "().get_id();\n";
  body << "}\n\n";

  body << name << "_id_of_" << tname << ' ';
  body << name << "_get_next_" << tname << '(';
  body << name << "_db *db, " << name << "_id_of_" << tname <<" id)\n{\n";
  body << convert.str();
  body << " return p->get_next_" << tname << "(" << name;
  body << "::id_of_" << tname << "(id)).get_id();\n";
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

  for (const auto &field: table.second.get_fields())
  {
   const std::string &fname = field.second.get_name();
   write_c_type(body, options, field.second.get_type());
   body << name << "_get_" << tname << '_' << fname << "(";
   body << name << "_db *db, ";
   body << name << "_id_of_" << tname;
   body << " id)\n{\n";
   body << convert.str();
   body << " return p->get_" << fname << '(' << name << "::id_of_" << tname << "(id))";
   if (field.second.get_type().get_type_id() == Type::type_id_t::string)
    body << ".c_str()";
   else if (field.second.get_type().get_type_id() == Type::type_id_t::reference)
    body << ".get_id()";
   body << ";\n";
   body << "}\n\n";
   body << "void " << name << "_set_" << tname << '_' << fname << "(";
   body << name << "_db *db, ";
   body << name << "_id_of_" << tname << " id, ";
   write_c_type(body, options, field.second.get_type());
   body << "value)\n{\n";
   body << convert.str();
   body << " p->set_" << fname << '(' << name << "::id_of_" << tname << "(id), ";
   if (field.second.get_type().get_type_id() == Type::type_id_t::reference)
   {
    body << name << "::id_of_";
    body << db.get_tables().find(field.second.get_type().get_table_id())->second.get_name();
    body << "(value)";
   }
   else
    body << "value";
   body << ");\n";
   body << "}\n\n";
  }
 }
}

}
