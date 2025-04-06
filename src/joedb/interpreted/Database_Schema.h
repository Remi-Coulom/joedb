#ifndef joedb_Database_Schema_declared
#define joedb_Database_Schema_declared

#include "joedb/Readable.h"
#include "joedb/Writable.h"
#include "joedb/interpreted/Table.h"

#include <map>

namespace joedb
{
 /// @ingroup interpreted
 class Database_Schema: public Readable, public Writable
 {
  protected:
   std::map<Table_Id, Table> tables;
   std::map<Table_Id, std::string> table_names;
   Table_Id current_table_id = Table_Id(0);

   static void check_identifier
   (
    const char *message,
    const std::string &name
   );

   const Table& get_table(Table_Id table_id) const;
   Table& get_table(Table_Id table_id);

  public:
   //
   // Readable override
   //
   const std::map<Table_Id, std::string> &get_tables() const override
   {
    return table_names;
   }

   const std::map<Field_Id, std::string> &get_fields
   (
    Table_Id table_id
   ) const override;

   const Type &get_field_type
   (
    Table_Id table_id,
    Field_Id field_id
   ) const override;

   const Compact_Freedom_Keeper &get_freedom(Table_Id table_id) const override;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   const type &get_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id\
   ) const override;
   #include "joedb/TYPE_MACRO.h"

   //
   // Writable override
   //
   void create_table(const std::string &name) override;
   void drop_table(Table_Id table_id) override;
   void rename_table(Table_Id table_id, const std::string &name) override;
   void add_field
   (
    Table_Id table_id,
    const std::string &name,
    Type type
   ) override;
   void drop_field(Table_Id table_id, Field_Id field_id) override;
   void rename_field(Table_Id, Field_Id, const std::string &name) override;

   virtual ~Database_Schema() override;
 };
}

#endif
