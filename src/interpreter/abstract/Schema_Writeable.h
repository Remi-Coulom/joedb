#ifndef joedb_Schema_Writeable_declared
#define joedb_Schema_Writeable_declared

#include "joedb/Writeable.h"
#include "joedb/Database.h"

namespace joedb
{
 class Schema_Writeable: public Writeable
 {
  protected:
   Database &db;
   bool error;

  public:
   Schema_Writeable(Database &db): db(db), error(false) {}

   bool is_good() const override {return !error;}

   void create_table(const std::string &name) override
   {
    error |= !db.create_table(name);
   }

   void drop_table(table_id_t table_id) override
   {
    error |= !db.drop_table(table_id);
   }

   void rename_table(table_id_t table_id,
                     const std::string &name) override
   {
    error |= !db.rename_table(table_id, name);
   }

   void add_field(table_id_t table_id,
                  const std::string &name,
                  Type type) override
   {
    error |= !db.add_field(table_id, name, type);
   }

   void drop_field(table_id_t table_id, field_id_t field_id) override
   {
    error |= !db.drop_field(table_id, field_id);
   }

   void rename_field(table_id_t table_id,
                     field_id_t field_id,
                     const std::string &name) override
   {
    error |= !db.rename_field(table_id, field_id, name);
   }

   void custom(const std::string &name) override
   {
    db.custom(name);
   }
 };
}

#endif
