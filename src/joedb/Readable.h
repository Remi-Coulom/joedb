#ifndef joedb_Readable_declared
#define joedb_Readable_declared

#include "joedb/Type.h"

#include <map>
#include <string>

namespace joedb
{
 class Compact_Freedom_Keeper;

 /// @ingroup joedb
 class Readable
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

   virtual const Compact_Freedom_Keeper &get_freedom(Table_Id table_id) const = 0;
   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   virtual const type &get_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id\
   ) const = 0;
   #include "joedb/TYPE_MACRO.h"

   // TODO: iterators to iterate over table rows?

   Table_Id find_table(const std::string &name) const;
   Field_Id find_field(Table_Id table_id, const std::string &name) const;
   const std::string &get_table_name(Table_Id table_id) const;
   const std::string &get_field_name
   (
    Table_Id table_id,
    Field_Id field_id
   ) const;
   Record_Id get_last_record_id(Table_Id table_id) const;
   bool is_used(Table_Id table_id, Record_Id record_id) const;

   virtual ~Readable();
 };
}

#endif
