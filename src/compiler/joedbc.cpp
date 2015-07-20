#include "Database.h"
#include "File.h"
#include "Interpreter.h"

#include <iostream>
#include <fstream>

/////////////////////////////////////////////////////////////////////////////
void write_type(std::ostream &out,
                const joedb::Database &db,
                joedb::Type type,
                bool return_type)
{
 switch (type.get_type_id())
 {
  case joedb::Type::type_id_t::null:
   out << "void";
  break;

  case joedb::Type::type_id_t::string:
   if (return_type)
    out << "const std::string &";
   else
    out << "std::string";
  break;

  case joedb::Type::type_id_t::int32:
   out << "int32_t";
  break;

  case joedb::Type::type_id_t::int64:
   out << "int64_t";
  break;

  case joedb::Type::type_id_t::reference:
  {
   const table_id_t referred = type.get_table_id();
   out << db.get_tables().find(referred)->second.get_name() << "_t";
  }
  break;
 }
}

/////////////////////////////////////////////////////////////////////////////
void generate_code(std::ostream &out,
                   const joedb::Database &db,
                   const char *dbname)
{
 char const * const types[joedb::Type::type_ids] =
 {
  0,
  "string",
  "int32",
  "int64",
  "reference"
 };

 char const * const cpp_types[joedb::Type::type_ids] =
 {
  0,
  "const std::string &",
  "int32_t ",
  "int64_t ",
  "record_id_t "
 };

 auto tables = db.get_tables();

 out << "#ifndef " << dbname << "_Database_declared\n";
 out << "#define " << dbname << "_Database_declared\n";
 out << R"RRR(
#include <string>
#include <cstdint>
#include <vector>
#include <cassert>

#include "File.h"
#include "Journal_File.h"
#include "Database.h"
#include "Schema_Listener.h"
#include "Freedom_Keeper.h"

)RRR";

 out << "namespace " << dbname << "\n{\n";
 out << " class Database;\n\n";

 for (auto table: tables)
 {
  const std::string &tname = table.second.get_name();
  out << " class " << tname << "_container;\n\n";
  out << " class " << tname << "_t\n {\n";
  out << "  friend class Database;\n";
  for (auto friend_table: tables)
   if (friend_table.first != table.first)
    out << "  friend class " << friend_table.second.get_name() << "_t;\n";
  out << "  friend class "  << tname << "_container;\n";
  out << "\n  private:\n";
  out << "   record_id_t id;\n";
  out << "   " << tname << "_t(record_id_t id): id(id) {}\n";
  out << "\n  public:\n";
  out << "   " << tname << "_t(): id(0) {}\n";
  out << "   bool is_null() const {return id == 0;}\n";
  out << " };\n";
  out << "\n struct " << tname << "_data: public joedb::EmptyRecord\n {\n";
  out << "  " << tname << "_data() {}\n";
  out << "  " << tname << "_data(bool f): joedb::EmptyRecord(f) {}\n";

  for (const auto &field: table.second.get_fields())
  {
   out << "  ";
   write_type(out, db, field.second.get_type(), false);
   out << ' ' << field.second.get_name() << ";\n";
  }

  out << " };\n\n";
 }

 out << " class Database: private joedb::Listener\n {\n";

 for (auto table: tables)
 {
  out << "  friend class "  << table.second.get_name() << "_t;\n";
  out << "  friend class "  << table.second.get_name() << "_container;\n";
 }

 out << R"RRR(
  private:
   joedb::File file;
   joedb::Journal_File journal;

)RRR";

 //
 // Vectors, and freedom keepers
 //
 for (auto table: tables)
 {
  const std::string &tname = table.second.get_name();
  out << "   joedb::Freedom_Keeper<" << tname << "_data> " << tname << "_FK;\n";
 }

 //
 // after_delete listener function
 //
 out << '\n';
 out << "   void after_delete(table_id_t table_id, record_id_t record_id) override\n";
 out << "   {\n";
 {
  bool first = true;
  for (auto table: tables)
  {
   out << "    ";
   if (first)
    first = false;
   else
    out << "else ";

   const std::string &name = table.second.get_name();
   out << "if (table_id == " << table.first << ")\n";
   out << "     " << name << "_FK.free(record_id + 1);\n";
  }
 }
 out << "   }\n";

 //
 // after_insert listener function
 //
 out << '\n';
 out << "   void after_insert(table_id_t table_id, record_id_t record_id) override\n";
 out << "   {\n";
 {
  bool first = true;
  for (auto table: tables)
  {
   out << "    ";
   if (first)
    first = false;
   else
    out << "else ";

   const std::string &name = table.second.get_name();
   out << "if (table_id == " << table.first << ")\n";
   out << "    {\n";
   out << "     while (" << name << "_FK.size() < record_id)\n";
   out << "      " << name << "_FK.push_back();\n";
   out << "     " << name << "_FK.use(record_id + 1);\n";
   out << "    }\n";
  }
 }
 out << "   }\n";

 //
 // after_update
 //
 {
  for (int type_id = 1;
       type_id < int(joedb::Type::type_ids);
       type_id++)
  {
   out << '\n';
   out << "   void after_update_" << types[type_id] << '\n';
   out << "   (\n";
   out << "    table_id_t table_id,\n";
   out << "    record_id_t record_id,\n";
   out << "    field_id_t field_id,\n";
   out << "    " << cpp_types[type_id] << "value\n";
   out << "   )\n";
   out << "   override\n";
   out << "   {\n";

   for (auto table: tables)
   {
    bool has_typed_field = false;

    for (auto field: table.second.get_fields())
     if (int(field.second.get_type().get_type_id()) == type_id)
     {
      has_typed_field = true;
      break;
     }

    if (has_typed_field)
    {
     out << "    if (table_id == " << table.first << ")\n";
     out << "    {\n";

     for (auto field: table.second.get_fields())
      if (int(field.second.get_type().get_type_id()) == type_id)
      {
       out << "     if (field_id == " << field.first << ")\n";
       out << "     {\n";
       out << "      " << table.second.get_name();
       out << "_FK.get_record(record_id + 1).";
       out << field.second.get_name() << " = value;\n";
       out << "      return;\n";
       out << "     }\n";
      }

     out << "     return;\n";
     out << "    }\n";
    }
   }

   out << "   }\n";
  }
 }

 //
 // Public stuff
 //
 out << R"RRR(
  public:
   Database(const char *file_name, bool read_only = false):
    file(file_name,
         read_only ? joedb::File::mode_t::read_existing :
                     joedb::File::mode_t::write_existing),
    journal(file)
   {
    if (is_good())
     journal.replay_log(*this);
   }

   joedb::Journal_File::state_t get_journal_state() const
   {
    return journal.get_state();
   }

   void commit() {file.commit();}
   void checkpoint() {journal.checkpoint();}

   bool is_good() const
   {
    return file.is_good() &&
           journal.get_state() == joedb::Journal_File::state_t::no_error;
   }
)RRR";

 for (auto table: tables)
 {
  out << '\n';
  const std::string &tname = table.second.get_name();

  out << "   " << tname << "_container get_" << tname << "_table() const;\n";
  out << "   " << tname << "_t new_" << tname << "()\n";
  out << "   {\n";
  out << "    " << tname << "_t result(" << tname << "_FK.allocate() - 1);\n";
  out << "    journal.after_insert(" << table.first << ", result.id);\n";
  out << "    return result;\n";
  out << "   }\n";
  out << '\n';

  //
  // new with all fields
  //
  out << "   " << tname << "_t new_" << tname << '\n';
  out << "   (\n    ";
  {
   bool first = true;

   for (const auto &field: table.second.get_fields())
   {
    const std::string &fname = field.second.get_name();

    if (first)
     first = false;
    else
     out << ",\n    ";

    write_type(out, db, field.second.get_type(), true);
    out << ' ' << fname;
   }

   out << '\n';
  }
  out << "   )\n";
  out << "   {\n";
  out << "    " << tname << "_t result(" << tname << "_FK.allocate() - 1);\n";
  out << "    journal.after_insert(" << table.first << ", result.id);\n";

  for (const auto &field: table.second.get_fields())
  {
   const std::string &fname = field.second.get_name();

   out << "    " << tname << "_FK.get_record(result.id + 1).";
   out << fname << " = " << fname << ";\n";
   out << "    journal.after_update_";
   out << types[int(field.second.get_type().get_type_id())];
   out << '(' << table.first << ", result.id, " << field.first << ", ";
   out << fname;
   if (field.second.get_type().get_type_id() ==
       joedb::Type::type_id_t::reference)
    out << ".id";
   out << ");\n";
  }

  out << "    return result;\n";
  out << "   }\n\n";

  //
  // Delete
  //
  out << "   void delete_record(" << tname << "_t record)\n";
  out << "   {\n";
  out << "    " << tname << "_FK.free(record.id + 1);\n";
  out << "    journal.after_delete(" << table.first << ", record.id);\n";
  out << "   }\n";

  //
  // getters and setters for each field
  //
  for (const auto &field: table.second.get_fields())
  {
   const std::string &fname = field.second.get_name();
   out << '\n';
   out << "   ";
   write_type(out, db, field.second.get_type(), true);
   out << " get_" << fname << "(" << tname << "_t record)\n";
   out << "   {\n";
   out << "    assert(!record.is_null());\n";
   out << "    return " << tname;
   out << "_FK.get_record(record.id + 1)." << fname << ";\n";
   out << "   }\n";

   out << "   void set_" << fname;
   out << "(" << tname << "_t record, ";
   write_type(out, db, field.second.get_type(), true);
   out << ' ' << fname << ")\n";
   out << "   {\n";
   out << "    assert(!record.is_null());\n";
   out << "    " << tname << "_FK.get_record(record.id + 1).";
   out << fname << " = " << fname << ";\n";
   out << "    journal.after_update_";
   out << types[int(field.second.get_type().get_type_id())];
   out << '(' << table.first << ", record.id, " << field.first << ", ";
   out << fname;
   if (field.second.get_type().get_type_id() ==
       joedb::Type::type_id_t::reference)
    out << ".id";
   out << ");\n";
   out << "   }\n";
  }
 }

 out << " };\n\n";

 for (auto table: tables)
 {
  const std::string &tname = table.second.get_name();
  out << " class " << tname << "_container\n";
  out << " {\n";
  out << "  friend class Database;\n";
  out << '\n';
  out << "  private:\n";
  out << "   const Database &db;\n";
  out << "   " << tname << "_container(const Database &db): db(db) {}\n";
  out << '\n';
  out << "  public:\n";
  out << "   class iterator\n";
  out << "   {\n";
  out << "    friend class " << tname << "_container;\n";

  out << "    private:\n";
  out << "     const joedb::Freedom_Keeper<" << tname << "_data> &fk;\n";
  out << "     size_t index;\n";
  out << "     iterator(const joedb::Freedom_Keeper<" << tname << "_data> &fk): fk(fk), index(0) {}\n";
  out << '\n';
  out << "    public:\n";
  out << "     bool operator!=(const iterator &i) const {return index != i.index;}\n";
  out << "     iterator &operator++() {index = fk.get_next(index); return *this;}\n";

  out << "     " << tname << "_t operator*() {return ";
  out << tname << "_t(index - 1);}\n";
  out << "   };\n";
  out << '\n';
  out << "   iterator begin() {return ++iterator(db." << tname << "_FK);}\n";
  out << "   iterator end() {return iterator(db." << tname << "_FK);}\n";
  out << " };\n";
  out << '\n';
  out << ' ' << tname << "_container Database::get_" << tname << "_table() const\n";
  out << " {\n";
  out << "  return " << tname << "_container(*this);\n";
  out << " }\n";
  out << '\n';
 }

 out << "}\n\n";
 out << "#endif\n";
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
 //
 // Open existing file from the command line
 //
 if (argc <= 2)
 {
  std::cerr << "Usage: " << argv[0] << " <file.joedbi> <namespace>\n";
  return 1;
 }

 std::ifstream file(argv[1]);
 if (!file.good())
 {
  std::cerr << "Error: could not open " << argv[1] << '\n';
  return 1;
 }

 //
 // Read the database
 //
 joedb::Database db;
 joedb::Interpreter interpreter(db);
 interpreter.main_loop(file, std::cerr);

 //
 // Generate code
 //
 generate_code(std::cout, db, argv[2]);

 return 0;
}
