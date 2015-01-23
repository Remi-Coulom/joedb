#ifndef joedb_Record_Declared
#define joedb_Record_Declared

#include <vector>
#include <deque>

#include "Value.h"

namespace joedb
{
 class Record
 {
  friend class Table;
  friend class RecordIterator;

  private:
   bool is_free;
   record_id_t next;
   record_id_t previous;
   std::vector<Value> values;

  public:
   Record(size_t n): is_free(true), values(n) {}
   const std::vector<Value> &get_values() const {return values;}
 };

 typedef std::deque<Record> RecordCollection;

 class RecordIterator
 {
  private:
   RecordCollection &records;
   record_id_t record_id;

  public:
   RecordIterator(RecordCollection &records, record_id_t record_id):
    records(records),
    record_id(record_id)
   {
   }

   record_id_t get_record_id() const {return record_id;}
   Record &operator*() {return records[record_id - 1];}
   void operator++() {record_id = records[record_id - 1].next;}
   bool operator!=(const RecordIterator &record_iterator) const
   {
    return record_id != record_iterator.record_id;
   }
 };
}

#endif
