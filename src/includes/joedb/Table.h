#ifndef joedb_Table_declared
#define joedb_Table_declared

#include <map>

#include "Field.h"
#include "Type.h"
#include "Freedom_Keeper.h"

namespace joedb
{
 class Database;

 class Table
 {
  friend class Database;

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
   void set_name(const std::string &new_name) {name = new_name;}
   const Freedom_Keeper<> &get_freedom() const {return freedom;}

   const std::map<field_id_t, Field> &get_fields() const {return fields;}
   field_id_t find_field(const std::string &name) const;
   field_id_t add_field(const std::string &name, const Type &type);
   bool drop_field(field_id_t field_id);

   bool delete_record(record_id_t record_id);
   bool insert_record(record_id_t record_id);

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   return_type get_##type_id(record_id_t rid, field_id_t fid) const\
   {\
    return fields.find(fid)->second.get_##type_id(rid);\
   }\
   bool update_##type_id(record_id_t record_id,\
                         field_id_t field_id,\
                         return_type value)\
   {\
    auto it = fields.find(field_id);\
    if (it == fields.end() || !freedom.is_used(record_id + 1))\
     return false;\
    it->second.set_##type_id(record_id, value);\
    return true;\
   }\
   bool update_vector_##type_id(record_id_t record_id,\
                                field_id_t field_id,\
                                record_id_t size,\
                                const type *value)\
   {\
    auto it = fields.find(field_id);\
    if (it == fields.end() ||\
        !freedom.is_used(record_id + 1) ||\
        !freedom.is_used(record_id + size))\
     return false;\
    it->second.set_vector_##type_id(record_id, size, value);\
    return true;\
   }
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
