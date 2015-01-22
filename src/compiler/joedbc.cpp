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
 out << "#ifndef " << dbname << "_database_declared\n";
 out << "#define " << dbname << "_database_declared\n";
 out << '\n';
 out << "#include <string>\n";
 out << "#include <cstdint>\n";
 out << '\n';
 out << "#include \"index_types.h\"\n";
 out << '\n';
 out << "namespace " << dbname << "\n{\n";
 out << " class database\n";
 out << " {\n";
 out << " };\n";

 auto tables = db.get_tables();

 for (auto table: tables)
 {
  const auto &fields = table.second.get_fields();

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
     out << "record_id_t";
    break;
   }
   out << ' ' << field.second.name << ";\n";
  }

  out << " };\n";
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
     schema_listener.get_error())
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
