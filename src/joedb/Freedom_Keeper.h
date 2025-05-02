#ifndef joedb_Freedom_Keeper_declared
#define joedb_Freedom_Keeper_declared

#include <cstddef>
#include <vector>

#include "joedb/error/assert.h"

namespace joedb
{
 /// @ingroup joedb
 class Freedom_Keeper
 {
  private: //////////////////////////////////////////////////////////////////
   struct Record
   {
    public:
     bool is_free;
     ptrdiff_t next;
     ptrdiff_t previous;

    public:
     Record() {}
     Record(bool is_free, ptrdiff_t next, ptrdiff_t previous):
      is_free(is_free),
      next(next),
      previous(previous)
     {}
   };

   ptrdiff_t used_count;
   std::vector<Record> records;
   enum {used_list = 0, free_list = 1};

  public: ///////////////////////////////////////////////////////////////////
   Freedom_Keeper(): used_count(0), records(2)
   {
    records[used_list].is_free = false;
    records[used_list].next = used_list;
    records[used_list].previous = used_list;

    records[free_list].is_free = true;
    records[free_list].next = free_list;
    records[free_list].previous = free_list;
   }

   bool is_empty() const {return used_count == 0;}
   ptrdiff_t get_used_count() const {return used_count;}
   ptrdiff_t size() const {return ptrdiff_t(records.size() - 2);}
   ptrdiff_t get_first_free() const {return records[free_list].next;}
   ptrdiff_t get_first_used() const {return records[used_list].next;}
   ptrdiff_t get_next(ptrdiff_t index) const {return records[index].next;}
   ptrdiff_t get_previous(ptrdiff_t index) const {return records[index].previous;}
   bool is_free(ptrdiff_t index) const {return records[index].is_free;}
   bool is_used(ptrdiff_t index) const
   {
    return index > 1 &&
           index < ptrdiff_t(records.size()) &&
           !records[index].is_free;
   }

   //////////////////////////////////////////////////////////////////////////
   ptrdiff_t get_free_record()
   {
    ptrdiff_t result = records[free_list].next;
    if (result == free_list)
    {
     push_back();
     result = records[free_list].next;
    }
    return result;
   }

   //////////////////////////////////////////////////////////////////////////
   ptrdiff_t allocate()
   {
    const ptrdiff_t result = get_free_record();
    use(result);
    return result;
   }

   //////////////////////////////////////////////////////////////////////////
   ptrdiff_t push_back()
   {
    const ptrdiff_t index = records.size();
    records.emplace_back(true, records[free_list].next, free_list);

    records[records[free_list].next].previous = index;
    records[free_list].next = index;

    return index;
   }

   //////////////////////////////////////////////////////////////////////////
   void resize(ptrdiff_t new_size)
   {
    while(size() < new_size)
     push_back();
   }

   //////////////////////////////////////////////////////////////////////////
   void use(ptrdiff_t index)
   {
    JOEDB_DEBUG_ASSERT(index > 1);
    JOEDB_DEBUG_ASSERT(index < records.size());
    JOEDB_DEBUG_ASSERT(records[index].is_free);

    Record &record = records[index];
    record.is_free = false;

    records[record.previous].next = record.next;
    records[record.next].previous = record.previous;

    record.previous = records[used_list].previous;
    record.next = used_list;

    records[record.previous].next = index;
    records[record.next].previous = index;

    used_count++;
   }

   //////////////////////////////////////////////////////////////////////////
   void free(ptrdiff_t index)
   {
    JOEDB_DEBUG_ASSERT(index > 1);
    JOEDB_DEBUG_ASSERT(index < records.size());
    JOEDB_DEBUG_ASSERT(!records[index].is_free);

    Record &record = records[index];
    record.is_free = true;

    records[record.previous].next = record.next;
    records[record.next].previous = record.previous;

    record.next = records[free_list].next;
    record.previous = 1;

    records[records[free_list].next].previous = index;
    records[free_list].next = index;

    used_count--;
   }
 };

 /// @ingroup joedb
 class Compact_Freedom_Keeper
 {
  private:
   bool compact;
   ptrdiff_t compact_used_size;
   ptrdiff_t compact_free_size;

   Freedom_Keeper fk;

   void lose_compactness()
   {
    compact = false;

    while (fk.size() < compact_free_size)
     fk.push_back();

    for (ptrdiff_t i = 0; i < compact_used_size; i++)
     fk.use(i + 2);
   }

  public:
   Compact_Freedom_Keeper():
    compact(true),
    compact_used_size(0),
    compact_free_size(0)
   {
   }

   ptrdiff_t size() const
   {
    if (compact)
     return compact_free_size;
    else
     return fk.size();
   }

   bool is_empty() const
   {
    if (compact)
     return compact_used_size == 0;
    else
     return fk.is_empty();
   }

   ptrdiff_t get_used_count() const
   {
    if (compact)
     return compact_used_size;
    else
     return fk.get_used_count();
   }

   bool is_used(ptrdiff_t index) const
   {
    if (compact)
     return index - 2 < compact_used_size;
    else
     return fk.is_used(index);
   }

   bool is_free(ptrdiff_t index) const
   {
    if (compact)
     return index - 2 >= compact_used_size;
    else
     return fk.is_free(index);
   }

   ptrdiff_t get_free_record()
   {
    if (compact)
    {
     if (compact_free_size == compact_used_size)
      ++compact_free_size;
     return compact_used_size + 2;
    }
    else
     return fk.get_free_record();
   }

   void use(ptrdiff_t index)
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

   void use_vector(ptrdiff_t index, ptrdiff_t size)
   {
    if
    (
     compact &&
     index == compact_used_size + 2 &&
     compact_used_size + size <= compact_free_size
    )
    {
     compact_used_size += size;
    }
    else
    {
     for (ptrdiff_t i = 0; i < size; i++)
      use(index + i);
    }
   }

   void free(ptrdiff_t index)
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

   ptrdiff_t push_back()
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

   ptrdiff_t get_first_free() const
   {
    if (compact)
    {
     if (compact_used_size == compact_free_size)
      return 1;
     else
      return compact_used_size + 2;
    }
    else
     return fk.get_first_free();
   }

   ptrdiff_t get_first_used() const
   {
    if (compact)
    {
     if (compact_used_size == 0)
      return 0;
     else
      return 2;
    }
    else
     return fk.get_first_used();
   }

   ptrdiff_t get_next(ptrdiff_t index) const
   {
    if (compact)
    {
     if (index == 0)
      return get_first_used();

     if (index == 1)
      return get_first_free();

     const ptrdiff_t result = index + 1;

     if (result == compact_used_size + 2)
      return 0;

     if (index == compact_free_size + 2)
      return 1;

     return result;
    }
    else
     return fk.get_next(index);
   }

   ptrdiff_t get_previous(ptrdiff_t index) const
   {
    if (compact)
    {
     if (index == 0)
     {
      if (compact_used_size == 0)
       return 0;
      else
       return compact_used_size + 1;
     }

     if (index == 1)
     {
      if (compact_used_size == compact_free_size)
       return 1;
      else
       return compact_free_size + 1;
     }

     const ptrdiff_t result = index - 1;

     if (result == 1)
      return 0;
     if (result == compact_used_size + 1)
      return 1;
     return index - 1;
    }
    else
     return fk.get_previous(index);
   }

   void resize(ptrdiff_t size)
   {
    if (compact)
    {
     if (compact_free_size < size)
      compact_free_size = size;
    }
    else
     fk.resize(size);
   }

   void append_vector(ptrdiff_t size)
   {
    if (compact && compact_free_size == compact_used_size)
    {
     compact_free_size += size;
     compact_used_size += size;
    }
    else
    {
     for (ptrdiff_t i = 0; i < size; i++)
      use(push_back());
    }
   }
 };
}

#endif
