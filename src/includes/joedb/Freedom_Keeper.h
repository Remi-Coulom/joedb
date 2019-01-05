#ifndef joedb_Freedom_Keeper_declared
#define joedb_Freedom_Keeper_declared

#include <cstddef>
#include <vector>

#include "joedb_assert.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class EmptyRecord
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   bool f;

  public:
   EmptyRecord() {}
   EmptyRecord(bool f): f(f) {}
   void set_free(bool new_f) {f = new_f;}
   bool is_free() const {return f;}
 };

 ////////////////////////////////////////////////////////////////////////////
 template<typename T = EmptyRecord> class Freedom_Keeper
 ////////////////////////////////////////////////////////////////////////////
 {
  private: //////////////////////////////////////////////////////////////////
   struct Record
   {
    public:
     T data;
     size_t next;
     size_t previous;

    public:
     Record() {}
     Record(bool f, size_t next, size_t previous):
      data(f),
      next(next),
      previous(previous)
     {}
   };

   size_t used_count;
   std::vector<Record> records;
   enum {used_list = 0, free_list = 1};

  public: ///////////////////////////////////////////////////////////////////
   Freedom_Keeper(): used_count(0), records(2)
   {
    records[used_list].data.set_free(false);
    records[used_list].next = used_list;
    records[used_list].previous = used_list;

    records[free_list].data.set_free(true);
    records[free_list].next = free_list;
    records[free_list].previous = free_list;
   }

   T &operator[](size_t index) {return records[index + 2].data;}
   const T &operator[](size_t index) const {return records[index + 2].data;}

   bool is_empty() const {return used_count == 0;}
   size_t get_used_count() const {return used_count;}
   size_t size() const {return records.size() - 2;}
   size_t get_first_free() const {return records[free_list].next;}
   size_t get_first_used() const {return records[used_list].next;}
   size_t get_next(size_t index) const {return records[index].next;}
   size_t get_previous(size_t index) const {return records[index].previous;}
   bool is_free(size_t index) const {return records[index].data.is_free();}
   bool is_used(size_t index) const
   {
    return index > 1 &&
           index < records.size() &&
           !records[index].data.is_free();
   }

   //////////////////////////////////////////////////////////////////////////
   size_t get_free_record()
   {
    size_t result = records[free_list].next;
    if (result == free_list)
    {
     push_back();
     result = records[free_list].next;
    }
    return result;
   }

   //////////////////////////////////////////////////////////////////////////
   size_t allocate()
   {
    size_t result = get_free_record();
    use(result);
    return result;
   }

   //////////////////////////////////////////////////////////////////////////
   size_t push_back()
   {
    const size_t index = records.size();
    records.emplace_back(true, records[free_list].next, free_list);

    records[records[free_list].next].previous = index;
    records[free_list].next = index;

    return index;
   }

   //////////////////////////////////////////////////////////////////////////
   void resize(size_t new_size)
   {
    while(size() < new_size)
     push_back();
   }

   //////////////////////////////////////////////////////////////////////////
   void use(size_t index)
   {
    JOEDB_ASSERT(index > 1);
    JOEDB_ASSERT(index < records.size());
    JOEDB_ASSERT(records[index].data.is_free());

    Record &record = records[index];
    record.data.set_free(false);

    records[record.previous].next = record.next;
    records[record.next].previous = record.previous;

    record.previous = records[used_list].previous;
    record.next = used_list;

    records[record.previous].next = index;
    records[record.next].previous = index;

    used_count++;
   }

   //////////////////////////////////////////////////////////////////////////
   void free(size_t index)
   {
    JOEDB_ASSERT(index > 1);
    JOEDB_ASSERT(index < records.size());
    JOEDB_ASSERT(!records[index].data.is_free());

    Record &record = records[index];
    record.data.set_free(true);

    records[record.previous].next = record.next;
    records[record.next].previous = record.previous;

    record.next = records[free_list].next;
    record.previous = 1;

    records[records[free_list].next].previous = index;
    records[free_list].next = index;

    used_count--;
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Compact_Freedom_Keeper
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   bool compact;
   size_t compact_used_size;
   size_t compact_free_size;

   Freedom_Keeper<> fk;

   void lose_compactness()
   {
    compact = false;

    while (fk.size() < compact_free_size)
     fk.push_back();

    for (size_t i = 0; i < compact_used_size; i++)
     fk.use(i + 2);
   }

  public:
   Compact_Freedom_Keeper():
    compact(true),
    compact_used_size(0),
    compact_free_size(0)
   {
   }

   size_t size() const
   {
    if (compact)
     return compact_free_size;
    else
     return fk.size();
   }

   bool is_used(size_t index) const
   {
    if (compact)
     return index - 2 < compact_used_size;
    else
     return fk.is_used(index);
   }

   bool is_free(size_t index) const
   {
    if (compact)
     return index - 2 >= compact_used_size;
    else
     return fk.is_free(index);
   }

   void use(size_t index)
   {
    if (compact)
    {
     if (index == compact_used_size + 2 && compact_used_size < compact_free_size)
      compact_used_size++;
     else
     {
      lose_compactness();
      fk.use(index);
     }
    }
    else
     fk.use(index);
   }

   void free(size_t index)
   {
    if (compact)
    {
     if (index == compact_used_size + 1 && index > 1)
      --compact_used_size;
     else
     {
      lose_compactness();
      fk.free(index);
     }
    }
    else
     fk.free(index);
   }

   size_t push_back()
   {
    if (compact)
     return ++compact_free_size + 1;
    else
     return fk.push_back();
   }

   bool is_compact() const
   {
    return compact;
   }
 };
}

#endif
