#ifndef joedb_Readable_declared
#define joedb_Readable_declared

#include "Type.h"

#include <map>
#include <vector>

namespace joedb
{
 class Table;

 class Readable
 {
  public:
   virtual record_id_t get_max_record_id() const = 0;
   virtual const std::map<table_id_t, Table> &get_tables() const = 0;
   virtual size_t get_current_table_id() const = 0;
   virtual table_id_t find_table(const std::string &name) const = 0;
   virtual field_id_t find_field(table_id_t table_id,
                                 const std::string &name) const = 0;
   virtual Type::type_id_t get_field_type(table_id_t table_id,
                                  field_id_t field_id) const = 0;

   virtual ~Readable() {}
 };
}

#endif
