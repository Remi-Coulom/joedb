#ifndef joedb_Schema_Writable_declared
#define joedb_Schema_Writable_declared

#include "Writable.h"
#include "Database.h"

namespace joedb
{
 class Schema_Writable: public Writable
 {
  protected:
   Database &db;

  public:
   Schema_Writable(Database &db): db(db) {}

   void create_table(const std::string &name) override
   {
    db.create_table(name);
   }

   void drop_table(Table_Id table_id) override
   {
    db.drop_table(table_id);
   }

   void rename_table(Table_Id table_id,
                     const std::string &name) override
   {
    db.rename_table(table_id, name);
   }

   void add_field(Table_Id table_id,
                  const std::string &name,
                  Type type) override
   {
    db.add_field(table_id, name, type);
   }

   void drop_field(Table_Id table_id, Field_Id field_id) override
   {
    db.drop_field(table_id, field_id);
   }

   void rename_field(Table_Id table_id,
                     Field_Id field_id,
                     const std::string &name) override
   {
    db.rename_field(table_id, field_id, name);
   }
 };
}

#endif
