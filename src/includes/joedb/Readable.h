#ifndef joedb_Readable_declared
#define joedb_Readable_declared

#include "Type.h"

#include <map>
#include <string>

namespace joedb
{
 typedef std::map<std::string, table_id_t> Table_Map;
 typedef std::map<std::string, field_id_t> Field_Map;

 class Readable
 {
  public:
   virtual const Table_Map &get_tables() const = 0;
   virtual const Field_Map &get_table_fields(table_id_t table_id) const = 0;
   virtual const Type &get_field_type(table_id_t table_id,
                                      field_id_t field_id) const = 0;

   virtual record_id_t get_last_record_id(table_id_t table_id) const = 0;
   virtual bool is_used(table_id_t table_id, record_id_t record_id) const = 0;
   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   virtual return_type get_##type_id(table_id_t table_id,\
                                     record_id_t record_id,\
                                     field_id_t field_id) const = 0;
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO

   // TODO: iterators to iterate over table rows?
   // Not convenient and efficient with virtual functions
   // Use templates instead of virtual functions?

   virtual ~Readable() {}

  public:
   table_id_t find_table(const std::string &name)
   {
    const Table_Map &tables = get_tables();
    auto it = tables.find(name);
    if (it == tables.end())
     return 0;
    else
     return it->second;
   }

   field_id_t find_field(table_id_t table_id, const std::string &name)
   {
    const Field_Map &fields = get_table_fields(table_id);
    auto it = fields.find(name);
    if (it == fields.end())
     return 0;
    else
     return it->second;
   }

   virtual const std::string &get_table_name(table_id_t table_id) const
   {
    for (const auto &table: get_tables())
     if (table.second == table_id)
      return table.first;
    static const std::string default_name = "__unknown_table__";
    return default_name;
   }

   virtual const std::string &get_field_name(table_id_t table_id,
                                             field_id_t field_id) const
   {
    for (const auto &field: get_table_fields(table_id))
     if (field.second == field_id)
      return field.first;
    static const std::string default_name = "__unknown_field__";
    return default_name;
   }
 };
}

#endif
