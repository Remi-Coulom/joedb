#ifndef joedb_Readable_declared
#define joedb_Readable_declared

#include "Type.h"
#include "Exception.h"

#include <map>
#include <string>

namespace joedb
{
 class Compact_Freedom_Keeper;

 ////////////////////////////////////////////////////////////////////////////
 class Readable
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   static const std::string default_table_name;
   static const std::string default_field_name;

  public:
   virtual const std::map<Table_Id, std::string> &get_tables() const = 0;
   virtual const std::map<Field_Id, std::string> &get_fields
   (
    Table_Id table_id
   ) const = 0;
   virtual const Type &get_field_type
   (
    Table_Id table_id,
    Field_Id field_id
   ) const = 0;

   virtual Record_Id get_last_record_id(Table_Id table_id) const = 0;
   virtual bool is_used(Table_Id table_id, Record_Id record_id) const = 0;
   virtual const Compact_Freedom_Keeper &get_freedom(Table_Id table_id) const = 0;
   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   virtual return_type get_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id\
   ) const = 0;\
   virtual const type &get_##type_id##_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id\
   ) const = 0;
   #include "joedb/TYPE_MACRO.h"

   // TODO: iterators to iterate over table rows?

   virtual ~Readable() = default;

   Table_Id find_table(const std::string &name) const
   {
    for (const auto &table: get_tables())
     if (table.second == name)
      return table.first;
    return 0;
   }

   Field_Id find_field(Table_Id table_id, const std::string &name) const
   {
    try
    {
     for (const auto &field: get_fields(table_id))
      if (field.second == name)
       return field.first;
    }
    catch (const Exception &)
    {
    }
    return 0;
   }

   const std::string &get_table_name(Table_Id table_id) const
   {
    const std::map<Table_Id, std::string> &tables = get_tables();
    auto it = tables.find(table_id);
    if (it == tables.end())
    {
     return default_table_name;
    }
    else
     return it->second;
   }

   const std::string &get_field_name
   (
    Table_Id table_id,
    Field_Id field_id
   ) const
   {
    try
    {
     const std::map<Field_Id, std::string> &fields = get_fields(table_id);
     auto it = fields.find(field_id);
     if (it != fields.end())
      return it->second;
    }
    catch (const Exception &)
    {
    }

    return default_field_name;
   }
 };
}

#endif
