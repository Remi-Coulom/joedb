#include "Database.h"
#include "File.h"
#include "JournalFile.h"
#include "SchemaListener.h"

#include <iostream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
void generate_code(std::ostream &out,
                   const Database &db,
                   const char *dbname)
{
 auto tables = db.get_tables();

 out << "#ifndef " << dbname << "_Database_declared\n";
 out << "#define " << dbname << "_Database_declared\n";
 out << R"RRR(
#include <string>
#include <cstdint>
#include <vector>

#include "index_types.h"
#include "File.h"
#include "JournalFile.h"

)RRR";

 out << "namespace " << dbname << "\n{\n";

 for (auto table: tables)
 {
  const auto &fields = table.second.get_fields();

  out << " class " << table.second.get_name() << "_t\n {\n";
  out << "  friend class Database;\n";
  out << "\n  private:\n";
  out << "   record_id_t id;\n";
  out << "   " << table.second.get_name() << "_t(record_id_t id): id(id) {}\n";
  out << "\n  public:\n";
  out << "   bool is_null() const {return id == 0;}\n";
  out << " };\n";

  out << "\n struct " << table.second.get_name() << "_data\n {\n";

  for (const auto &field: fields)
  {
   out << "  ";

   switch (field.second.type.get_type_id())
   {
    case Type::type_id_t::null:
     out << "void";
    break;

    case Type::type_id_t::string:
     out << "std::string";
    break;

    case Type::type_id_t::int32:
     out << "int32_t";
    break;

    case Type::type_id_t::int64:
     out << "int64_t";
    break;

    case Type::type_id_t::reference:
    {
     const table_id_t referred = field.second.type.get_table_id();
     out << db.get_tables().find(referred)->second.get_name() << "_t";
    }
    break;
   }
   out << ' ' << field.second.name << ";\n";
  }

  out << " };\n\n";
 }

 out << R"RRR( class Database: private joedb::Listener
 {
  private:
   joedb::File file;
   joedb::JournalFile journal;

)RRR";

 for (auto table: tables)
 {
  const std::string &name = table.second.get_name();
  out << "   std::vector<" << name << "_data> " << name << "_table;\n";
 }

 out << R"RRR(
  public:
   Database(const char *file_name, bool read_only = false):
    file(file_name,
         read_only ? joedb::File::mode_t::read_existing :
                     joedb::File::mode_t::write_existing),
    journal(file)
   {
   }

   bool is_good() const
   {
    return file.is_good() &&
           journal.get_state() != joedb::JournalFile::state_t::no_error;
   }
 };
)RRR";

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

 File file(argv[1], File::mode_t::read_existing);
 if (!file.is_good())
 {
  std::cerr << "Error: could not open " << argv[1] << '\n';
  return 1;
 }

 //
 // Read the database schema
 //
 JournalFile journal(file);
 Database db;
 SchemaListener schema_listener(db);
 journal.replay_log(schema_listener);
 if (journal.get_state() != JournalFile::state_t::no_error ||
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
