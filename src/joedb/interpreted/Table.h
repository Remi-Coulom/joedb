#ifndef joedb_Table_declared
#define joedb_Table_declared

#include <map>

#include "joedb/interpreted/Field.h"
#include "joedb/Type.h"
#include "joedb/error/Exception.h"
#include "joedb/Freedom_Keeper.h"

namespace joedb
{
 class Database;

 /// @ingroup interpreted
 class Table
 {
  friend class Database_Schema;

  private:
   std::map<Field_Id, Field> fields;
   std::map<Field_Id, std::string> field_names;
   Field_Id current_field_id = Field_Id(0);

   Compact_Freedom_Keeper freedom;

   auto find_field_from_id(Field_Id field_id)
   {
    const auto it = fields.find(field_id);
    if (it == fields.end())
     throw Exception("invalid field_id");
    return it;
   }

   auto find_field_from_id(Field_Id field_id) const
   {
    return const_cast<Table *>(this)->find_field_from_id(field_id);
   }

   void check_record_id(Record_Id record_id)
   {
    if (!freedom.is_used(to_underlying(record_id) + 1))
     throw Exception("update: invalid record_id");
   }

  public:
   const Compact_Freedom_Keeper &get_freedom() const {return freedom;}

   const std::map<Field_Id, Field> &get_fields() const {return fields;}
   Field_Id find_field(const std::string &name) const;
   void add_field(const std::string &name, const Type &type);
   void drop_field(Field_Id field_id);

   void delete_record(Record_Id record_id);
   void insert_record(Record_Id record_id);
   void insert_vector(Record_Id record_id, size_t size);

   size_t get_storage_capacity() const {return freedom.size();}

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   return_type get_##type_id(Record_Id rid, Field_Id fid) const\
   {\
    return find_field_from_id(fid)->second.get_##type_id(rid);\
   }\
   void update_##type_id(Record_Id record_id,\
                         Field_Id field_id,\
                         return_type value)\
   {\
    const auto it = find_field_from_id(field_id);\
    check_record_id(record_id);\
    it->second.set_##type_id(record_id, value);\
   }\
   void update_vector_##type_id(Record_Id record_id,\
                                Field_Id field_id,\
                                size_t size,\
                                const type *value)\
   {\
    const auto it = find_field_from_id(field_id);\
    check_record_id(record_id);\
    check_record_id(record_id + size - 1);\
    it->second.set_vector_##type_id(record_id, size, value);\
   }\
   type *get_own_##type_id##_storage(Record_Id record_id, Field_Id field_id)\
   {\
    const auto it = find_field_from_id(field_id);\
    check_record_id(record_id);\
    return it->second.get_own_##type_id##_storage(record_id);\
   }\
   const type *get_own_##type_id##_storage(Record_Id record_id, Field_Id field_id) const\
   {\
    return const_cast<Table *>(this)->get_own_##type_id##_storage(record_id, field_id);\
   }
   #include "joedb/TYPE_MACRO.h"
 };
}

#endif
