#ifndef joedb_Readable_declared
#define joedb_Readable_declared

#include "Type.h"

#include <map>
#include <string>

namespace joedb
{
 class Readable
 {
  public:
   virtual const std::map<table_id_t, std::string> &get_tables() const = 0;
   virtual const std::map<field_id_t, std::string> &get_fields(table_id_t table_id) const = 0;
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
   table_id_t find_table(const std::string &name) const
   {
    for (const auto &table: get_tables())
     if (table.second == name)
      return table.first;
    return 0;
   }

   field_id_t find_field(table_id_t table_id, const std::string &name) const
   {
    try
    {
     for (const auto &field: get_fields(table_id))
      if (field.second == name)
       return field.first;
    }
    catch (std::runtime_error &e)
    {
    }
    return 0;
   }

   const std::string &get_table_name(table_id_t table_id) const
   {
    const std::map<table_id_t, std::string> &tables = get_tables();
    auto it = tables.find(table_id);
    if (it == tables.end())
    {
     static const std::string default_name = "__unknown_table__";
     return default_name;
    }
    else
     return it->second;
   }

   const std::string &get_field_name(table_id_t table_id,
                                     field_id_t field_id) const
   {
    try
    {
     const std::map<field_id_t, std::string> &fields = get_fields(table_id);
     auto it = fields.find(field_id);
     if (it != fields.end())
      return it->second;
    }
    catch (std::runtime_error &e)
    {
    }

    static const std::string default_name = "__unknown_field__";
    return default_name;
   }
 };
}

#endif
