#ifndef joedb_Readable_Multiplexer_declared
#define joedb_Readable_Multiplexer_declared

#include "Multiplexer.h"
#include "Readable_Writable.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

namespace joedb
{
 class Readable_Multiplexer: public Readable_Writable, public Multiplexer
 {
  private:
   Readable &readable;

  public:
   Readable_Multiplexer(Readable_Writable &readable_writable):
    readable(readable_writable)
   {
    add_writable(readable_writable);
   }

   const std::map<Table_Id, std::string> &get_tables() const override
   {
    return readable.get_tables();
   }

   const std::map<Table_Id, std::string> &get_fields(Table_Id table_id) const override
   {
    return readable.get_fields(table_id);
   }

   const Type &get_field_type
   (
    Table_Id table_id,
    Field_Id field_id
   ) const override
   {
    return readable.get_field_type(table_id, field_id);
   }

   Record_Id get_last_record_id(Table_Id table_id) const override
   {
    return readable.get_last_record_id(table_id);
   }

   bool is_used(Table_Id table_id, Record_Id record_id) const override
   {
    return readable.is_used(table_id, record_id);
   }

   const Compact_Freedom_Keeper &get_freedom(Table_Id table_id) const override
   {
    return readable.get_freedom(table_id);
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   return_type get_##type_id(Table_Id table_id,\
                             Record_Id record_id,\
                             Field_Id field_id) const override\
   {\
    return readable.get_##type_id(table_id, record_id, field_id);\
   }\
   const type &get_##type_id##_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id\
   ) const override\
   {\
    return readable.get_##type_id##_storage(table_id, record_id, field_id);\
   }

   #include "joedb/TYPE_MACRO.h"
 };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
