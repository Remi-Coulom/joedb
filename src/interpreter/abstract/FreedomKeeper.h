#ifndef joedb_FreedomKeeper_declared
#define joedb_FreedomKeeper_declared

#include <cstddef>
#include <deque>
#include <cassert>

namespace joedb
{
 class FreedomKeeper
 {
  private: //////////////////////////////////////////////////////////////////
   struct Record
   {
    bool is_free;
    size_t next;
    size_t previous;
   };

   std::deque<Record> records;

   Record &used_list;
   Record &free_list;

  public: ///////////////////////////////////////////////////////////////////
   FreedomKeeper():
    records(2),
    used_list(records[0]),
    free_list(records[1])
   {
    used_list.is_free = false;
    used_list.next = 0;
    used_list.previous = 0;

    free_list.is_free = true;
    free_list.next = 1;
    free_list.previous = 1;
   }

   size_t size() const {return records.size() - 2;}
   size_t get_first_free() const {return free_list.next;}
   size_t get_first_used() const {return used_list.next;}
   size_t get_next(size_t index) const {return records[index].next;}
   bool is_free(size_t index) const {return records[index].is_free;}

   //////////////////////////////////////////////////////////////////////////
   void push_back()
   {
    const size_t index = records.size();
    records.push_back({true, free_list.next, 1});

    records[free_list.next].previous = index;
    free_list.next = index;
   }

   //////////////////////////////////////////////////////////////////////////
   void use(size_t index)
   {
    assert(index > 1);
    assert(index < records.size());
    assert(records[index].is_free);

    Record &record = records[index];
    record.is_free = false;

    records[record.previous].next = record.next;
    records[record.next].previous = record.previous;

    record.next = used_list.next;
    record.previous = 0;

    records[used_list.next].previous = index;
    used_list.next = index;
   }

   //////////////////////////////////////////////////////////////////////////
   void free(size_t index)
   {
    assert(index > 1);
    assert(index < records.size());
    assert(!records[index].is_free);

    Record &record = records[index];
    record.is_free = true;

    records[record.previous].next = record.next;
    records[record.next].previous = record.previous;

    record.next = free_list.next;
    record.previous = 1;

    records[free_list.next].previous = index;
    free_list.next = index;
   }
 };
}

#endif
