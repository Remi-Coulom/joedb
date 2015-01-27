#include "Database.h"
#include "File.h"
#include "JournalFile.h"
#include "SchemaListener.h"

#include <iostream>

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
 auto tables = db.get_tables();

 out << "#ifndef " << dbname << "_Database_declared\n";
 out << "#define " << dbname << "_Database_declared\n";
 out << R"RRR(
#include <string>
#include <cstdint>
#include <vector>
#include <cassert>

#include "File.h"
#include "JournalFile.h"
#include "Database.h"
#include "SchemaListener.h"
#include "FreedomKeeper.h"

)RRR";

 out << "namespace " << dbname << "\n{\n";
 out << " class Database;\n\n";

 for (auto table: tables)
 {
  out << " class " << table.second.get_name() << "_container;\n\n";
  out << " class " << table.second.get_name() << "_t\n {\n";
  out << "  friend class Database;\n";
  out << "  friend class "  << table.second.get_name() << "_container;\n";
  out << "\n  private:\n";
  out << "   record_id_t id;\n";
  out << "   " << table.second.get_name() << "_t(record_id_t id): id(id) {}\n";
  out << "\n  public:\n";
  out << "   " << table.second.get_name() << "_t(): id(0) {}\n";
  out << "   bool is_null() const {return id == 0;}\n";

  for (const auto &field: table.second.get_fields())
  {
   out << "   ";
   write_type(out, db, field.second.get_type(), true);
   out << " get_" << field.second.get_name() << "(const Database &db);\n";
  }

  out << " };\n";

  out << "\n struct " << table.second.get_name() << "_data\n {\n";

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
   joedb::JournalFile journal;

)RRR";

 //
 // Vectors, and freedom keepers
 //
 for (auto table: tables)
 {
  const std::string &name = table.second.get_name();
  out << "   std::vector<" << name << "_data> " << name << "_table;\n";
  out << "   joedb::FreedomKeeper " << name << "_FK;\n";
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
   out << "     " << name << "_table.resize(record_id);\n";
   out << "     while (" << name << "_FK.size() < record_id)\n";
   out << "      " << name << "_FK.push_back();\n";
   out << "    }\n";
  }
 }
 out << "   }\n";

 //
 // after_update
 //
 out << R"RRR(
#define AFTER_UPDATE(return_type, type_id)\
   void after_update_##type_id(table_id_t table_id,\
                               record_id_t record_id,\
                               field_id_t field_id,\
                               return_type value)\
   {\
   }

   AFTER_UPDATE(const std::string &, string)
   AFTER_UPDATE(int32_t, int32)
   AFTER_UPDATE(int64_t, int64)
   AFTER_UPDATE(record_id_t, reference)

#undef AFTER_UPDATE
)RRR";

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

   joedb::JournalFile::state_t get_journal_state() const
   {
    return journal.get_state();
   }

   bool is_good() const
   {
    return file.is_good() &&
           journal.get_state() == joedb::JournalFile::state_t::no_error;
   }

)RRR";

 for (auto table: tables)
 {
  const std::string &tname = table.second.get_name();
  out << "   " << tname << "_container get_" << tname << "_table() const;\n";
 }

 out << "\n};\n";

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

  out << R"RRR(
    private:
     const joedb::FreedomKeeper &fk;
     size_t index;
     iterator(const joedb::FreedomKeeper &fk): fk(fk), index(1) {}

    public:
     bool operator!=(const iterator &i) const {return index != i.index;}
     iterator &operator++() {index = fk.get_next(index); return *this;}
)RRR";

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
  out << " }\n\n";

  for (const auto &field: table.second.get_fields())
  {
   out << ' ';
   write_type(out, db, field.second.get_type(), true);
   out << ' ' << tname << "_t::";
   out << "get_" << field.second.get_name() << "(const Database &db)\n";
   out << " {\n";
   out << "  assert(!is_null());\n";
   out << "  return db." << tname;
   out << "_table[id - 1]." << field.second.get_name() << ";\n";
   out << " }\n";
  }
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
  std::cerr << "Usage: " << argv[0] << " <file.joedb> <namespace>\n";
  return 1;
 }

 joedb::File file(argv[1], joedb::File::mode_t::read_existing);
 if (!file.is_good())
 {
  std::cerr << "Error: could not open " << argv[1] << '\n';
  return 1;
 }

 //
 // Read the database schema
 //
 joedb::JournalFile journal(file);
 joedb::Database db;
 joedb::SchemaListener schema_listener(db);
 journal.replay_log(schema_listener);
 if (journal.get_state() != joedb::JournalFile::state_t::no_error ||
     !schema_listener.is_good())
 {
  std::cerr << "Error reading database\n";
  return 1;
 }

 //
 // Generate code
 //
 generate_code(std::cout, db, argv[2]);

 return 0;
}
