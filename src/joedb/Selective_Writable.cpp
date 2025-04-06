#include "joedb/Selective_Writable.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Selective_Writable::Selective_Writable(Writable &writable, Mode mode):
 ////////////////////////////////////////////////////////////////////////////
  writable(writable),
  mode(mode),
  blob_found(false)
 {
 }

 //
 // Schema events
 //
 void Selective_Writable::create_table(const std::string &name)
 {
  if (mode & schema)
   writable.create_table(name);
 }

 void Selective_Writable::drop_table(Table_Id table_id)
 {
  if (mode & schema)
   writable.drop_table(table_id);
 }

 void Selective_Writable::rename_table(Table_Id table_id,
                   const std::string &name)
 {
  if (mode & schema)
   writable.rename_table(table_id, name);
 }

 void Selective_Writable::add_field(Table_Id table_id,
                const std::string &name,
                Type type)
 {
  if (mode & schema)
   writable.add_field(table_id, name, type);
 }

 void Selective_Writable::drop_field(Table_Id table_id,
                 Field_Id field_id)
 {
  if (mode & schema)
   writable.drop_field(table_id, field_id);
 }

 void Selective_Writable::rename_field(Table_Id table_id,
                   Field_Id field_id,
                   const std::string &name)
 {
  if (mode & schema)
   writable.rename_field(table_id, field_id, name);
 }

 void Selective_Writable::custom(const std::string &name)
 {
  if (mode & schema)
   writable.custom(name);
 }

 //
 // Informative events
 //
 void Selective_Writable::comment(const std::string &comment)
 {
  if (mode & information)
   writable.comment(comment);
 }

 void Selective_Writable::timestamp(int64_t timestamp)
 {
  if (mode & information)
   writable.timestamp(timestamp);
 }

 void Selective_Writable::valid_data()
 {
  if (mode & information)
   writable.valid_data();
 }

 void Selective_Writable::checkpoint(Commit_Level commit_level)
 {
  writable.checkpoint(commit_level);
 }

 //
 // data events
 //
 void Selective_Writable::insert_into(Table_Id table_id, Record_Id record_id)
 {
  if (mode & data)
   writable.insert_into(table_id, record_id);
 }

 void Selective_Writable::insert_vector(Table_Id table_id,
                    Record_Id record_id,
                    size_t size)
 {
  if (mode & data)
   writable.insert_vector(table_id, record_id, size);
 }

 void Selective_Writable::delete_from(Table_Id table_id, Record_Id record_id)
 {
  if (mode & data)
   writable.delete_from(table_id, record_id);
 }

 #define TYPE_MACRO(type, return_type, type_id, R, W)\
 void Selective_Writable::update_##type_id(Table_Id table_id,\
                       Record_Id record_id,\
                       Field_Id field_id,\
                       return_type value)\
 {\
  if (mode & data)\
   writable.update_##type_id(table_id, record_id, field_id, value);\
 }
 #include "joedb/TYPE_MACRO.h"

 void Selective_Writable::on_blob(Blob blob)
 {
  blob_found = true;
 }
}
