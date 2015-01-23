#ifndef joedb_Table_declared
#define joedb_Table_declared

#include <map>
#include <deque>

#include "Field.h"
#include "Type.h"
#include "Record.h"
#include "index_types.h"

namespace joedb
{
 class Table
 {
  private:
   std::string name;

   std::map<field_id_t, Field> fields;
   field_id_t current_field_id;

   RecordCollection records;
   record_id_t first_record;
   record_id_t first_free_record;

  public:
   Table(const std::string &name):
    name(name),
    current_field_id(0),
    first_record(0),
    first_free_record(0)
   {
   }

   const std::string &get_name() const {return name;}

   const std::map<field_id_t, Field> &get_fields() const {return fields;}

   RecordIterator begin() {return RecordIterator(records, first_record);}

   field_id_t find_field(const std::string &name) const;
   field_id_t add_field(const std::string &name, const Type &type);
   bool drop_field(field_id_t field_id);
   bool delete_record(record_id_t record_id);
   bool insert_record(record_id_t record_id);
   bool update(record_id_t record_id, field_id_t field_id, const Value &value);
 };
}

#endif
