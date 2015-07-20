#ifndef joedb_Table_declared
#define joedb_Table_declared

#include <map>

#include "Field.h"
#include "Type.h"
#include "Freedom_Keeper.h"

namespace joedb
{
 class Table
 {
  private:
   std::string name;

   std::map<field_id_t, Field> fields;
   field_id_t current_field_id;

   Freedom_Keeper<> freedom;

  public:
   Table(const std::string &name):
    name(name),
    current_field_id(0)
   {
   }

   const std::string &get_name() const {return name;}
   const Freedom_Keeper<> &get_freedom() const {return freedom;}

   const std::map<field_id_t, Field> &get_fields() const {return fields;}
   field_id_t find_field(const std::string &name) const;
   field_id_t add_field(const std::string &name, const Type &type);
   bool drop_field(field_id_t field_id);

   bool delete_record(record_id_t record_id);
   bool insert_record(record_id_t record_id);

#define TABLE_GET(return_type, type_id)\
   return_type get_##type_id(record_id_t rid, field_id_t fid) const\
   {\
    return fields.find(fid)->second.get_##type_id(rid);\
   }\

   TABLE_GET(const std::string &, string)
   TABLE_GET(int32_t, int32)
   TABLE_GET(int64_t, int64)
   TABLE_GET(record_id_t, reference)

#undef TABLE_GET

#define TABLE_UPDATE(return_type, type_id)\
   bool update_##type_id(record_id_t record_id,\
                         field_id_t field_id,\
                         return_type value)\
   {\
    auto it = fields.find(field_id);\
    if (it == fields.end() || !freedom.is_used(record_id + 1))\
     return false;\
    it->second.set_##type_id(record_id, value);\
    return true;\
   }

   TABLE_UPDATE(const std::string &, string)
   TABLE_UPDATE(int32_t, int32)
   TABLE_UPDATE(int64_t, int64)
   TABLE_UPDATE(record_id_t, reference)

#undef TABLE_UPDATE
 };
}

#endif
