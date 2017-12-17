#include "merge.h"
#include "Database.h"

/////////////////////////////////////////////////////////////////////////////
void joedb::merge(Database &merged, const Database &db)
/////////////////////////////////////////////////////////////////////////////
{
 std::map<Table_Id, Record_Id> offset;

 //
 // First loop over tables to fill the offset map
 //
 for (auto table: merged.get_tables())
 {
  const Table_Id table_id = table.first;
  const Record_Id last_record_id = merged.get_last_record_id(table_id);
  offset[table_id] = last_record_id;
 }

 //
 // Second loop to copy data, with added offset
 //
 for (auto table: merged.get_tables())
 {
  const Table_Id table_id = table.first;
  const Record_Id last_record_id = db.get_last_record_id(table_id);
  for (Record_Id record_id = 1; record_id <= last_record_id; record_id++)
   if (db.is_used(table_id, record_id))
   {
    merged.insert_into(table_id, record_id + offset[table_id]);
   }
 }
}
