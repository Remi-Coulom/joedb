#ifndef joedb_FreedomKeeper_declared
#define joedb_FreedomKeeper_declared

#include <cstddef>
#include <vector>
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

   std::vector<Record> records;
   enum {used_list = 0, free_list = 1};

  public: ///////////////////////////////////////////////////////////////////
   FreedomKeeper(): records(2)
   {
    records[used_list].is_free = false;
    records[used_list].next = used_list;
    records[used_list].previous = used_list;

    records[free_list].is_free = true;
    records[free_list].next = free_list;
    records[free_list].previous = free_list;
   }

   size_t size() const {return records.size() - 2;}
   size_t get_first_free() const {return records[free_list].next;}
   size_t get_first_used() const {return records[used_list].next;}
   size_t get_next(size_t index) const {return records[index].next;}
   bool is_free(size_t index) const {return records[index].is_free;}
   bool is_used(size_t index) const
   {
    return index > 1 && index < records.size() && !records[index].is_free;
   }

   //////////////////////////////////////////////////////////////////////////
   size_t allocate()
   {
    size_t result = records[free_list].next;
    if (result == free_list)
    {
     push_back();
     result = records[free_list].next;
    }
    use(result);
    return result;
   }

   //////////////////////////////////////////////////////////////////////////
   void push_back()
   {
    const size_t index = records.size();
    records.push_back({true, records[free_list].next, 1});

    records[records[free_list].next].previous = index;
    records[free_list].next = index;
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

    record.next = records[used_list].next;
    record.previous = 0;

    records[records[used_list].next].previous = index;
    records[used_list].next = index;
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

    record.next = records[free_list].next;
    record.previous = 1;

    records[records[free_list].next].previous = index;
    records[free_list].next = index;
   }
 };
}

#endif
