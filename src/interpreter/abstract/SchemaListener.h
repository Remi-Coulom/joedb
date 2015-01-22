#ifndef joedb_SchemaListener_declared
#define joedb_SchemaListener_declared

#include "Listener.h"
#include "Database.h"

namespace joedb
{
 class SchemaListener: public Listener
 {
  protected:
   Database &db;
   bool error;

  public:
   SchemaListener(Database &db): db(db), error(false) {}

   bool is_good() const override {return !error;}

   void after_create_table(const std::string &name) override
   {
    error |= !db.create_table(name);
   }

   void after_drop_table(table_id_t table_id) override
   {
    error |= !db.drop_table(table_id);
   }

   void after_add_field(table_id_t table_id,
                        const std::string &name,
                        Type type) override
   {
    error |= !db.add_field(table_id, name, type);
   }

   void after_drop_field(table_id_t table_id, field_id_t field_id) override
   {
    error |= !db.drop_field(table_id, field_id);
   }
 };
}

#endif
