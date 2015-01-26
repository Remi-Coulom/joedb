#ifndef joedb_Table_declared
#define joedb_Table_declared

#include <map>
#include <deque>
#include <set>

#include "Field.h"
#include "Type.h"
#include "index_types.h"

namespace joedb
{
 class Table
 {
  private:
   std::string name;

   std::map<field_id_t, Field> fields;
   field_id_t current_field_id;

   std::map<field_id_t, void *> columns;
   std::deque<bool> is_free;

  public:
   Table(const std::string &name):
    name(name),
    current_field_id(0)
   {
   }

   const std::string &get_name() const {return name;}

   const std::deque<bool> &get_free_flags() const {return is_free;}

   const std::map<field_id_t, Field> &get_fields() const {return fields;}
   field_id_t find_field(const std::string &name) const;
   field_id_t add_field(const std::string &name, const Type &type);
   bool drop_field(field_id_t field_id);

   bool delete_record(record_id_t record_id);
   bool insert_record(record_id_t record_id);

   const std::string &get_string(record_id_t rid, field_id_t fid) const
   {
    return fields.find(fid)->second.get_string(rid);
   }

   int32_t get_int32(record_id_t rid, field_id_t fid) const;
   int64_t get_int64(record_id_t rid, field_id_t fid) const;
   record_id_t get_reference(record_id_t rid, field_id_t fid) const;

   // TODO: use a template?
   bool update_string(record_id_t record_id,
                      field_id_t field_id,
                      const std::string &value)
   {
    auto it = fields.find(field_id);
    if (it == fields.end())
     return false;
    it->second.set_string(record_id, value);
    return true;
   }

   bool update_int32(record_id_t record_id,
                     field_id_t field_id,
                     int32_t value);
   bool update_int64(record_id_t record_id,
                     field_id_t field_id,
                     int64_t value);
   bool update_reference(record_id_t record_id,
                         field_id_t field_id,
                         record_id_t value);
 };
}

#endif
