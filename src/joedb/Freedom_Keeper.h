#ifndef joedb_Freedom_Keeper_declared
#define joedb_Freedom_Keeper_declared

#include "joedb/index_types.h"
#include "joedb/error/assert.h"

#include <vector>

namespace joedb
{
 /// @ingroup joedb
 class Freedom_Keeper
 {
  public:
   virtual bool is_empty() const = 0;
   virtual index_t get_used_count() const = 0;
   virtual index_t size() const = 0;
   virtual index_t get_first_free() const = 0;
   virtual index_t get_first_used() const = 0;
   virtual index_t get_next(index_t index) const = 0;
   virtual index_t get_previous(index_t index) const = 0;
   virtual bool is_free(index_t index) const = 0;
   virtual bool is_used(index_t index) const = 0;
   virtual bool is_compact() const = 0;

   virtual index_t get_free_record() = 0;
   virtual index_t push_back() = 0;
   virtual void resize(index_t new_size) = 0;
   virtual bool use(index_t index) = 0;
   virtual bool free(index_t index) = 0;

   virtual bool use_vector(index_t index, index_t size)
   {
    for (index_t i = 0; i < size; i++)
     use(index + i);
    return true;
   }

   virtual bool append_vector(index_t size)
   {
    for (index_t i = 0; i < size; i++)
     use(push_back());
    return true;
   }

   virtual ~Freedom_Keeper() = default;
 };

 /// @ingroup joedb
 class List_Freedom_Keeper: public Freedom_Keeper
 {
  private: //////////////////////////////////////////////////////////////////
   struct Record
   {
    public:
     bool is_free;
     index_t next;
     index_t previous;

    public:
     Record() {}
     Record(bool is_free, index_t next, index_t previous):
      is_free(is_free),
      next(next),
      previous(previous)
     {}
   };

   index_t used_count;
   std::vector<Record> records;
   enum {used_list = 0, free_list = 1};

  public: ///////////////////////////////////////////////////////////////////
   List_Freedom_Keeper(): used_count(0), records(2)
   {
    records[used_list].is_free = false;
    records[used_list].next = used_list;
    records[used_list].previous = used_list;

    records[free_list].is_free = true;
    records[free_list].next = free_list;
    records[free_list].previous = free_list;
   }

   bool is_empty() const override {return used_count == 0;}
   index_t get_used_count() const override {return used_count;}
   index_t size() const override {return index_t(records.size() - 2);}
   index_t get_first_free() const override {return records[free_list].next;}
   index_t get_first_used() const override {return records[used_list].next;}
   index_t get_next(index_t index) const override {return records[index].next;}
   index_t get_previous(index_t index) const override {return records[index].previous;}
   bool is_free(index_t index) const override {return records[index].is_free;}
   bool is_used(index_t index) const override
   {
    return index > 1 &&
           index < index_t(records.size()) &&
           !records[index].is_free;
   }
   bool is_compact() const override {return size() == used_count;}

   //////////////////////////////////////////////////////////////////////////
   index_t get_free_record() override
   {
    index_t result = records[free_list].next;
    if (result == free_list)
    {
     push_back();
     result = records[free_list].next;
    }
    return result;
   }

   //////////////////////////////////////////////////////////////////////////
   index_t push_back() override
   {
    const index_t index = records.size();
    records.emplace_back(true, records[free_list].next, free_list);

    records[records[free_list].next].previous = index;
    records[free_list].next = index;

    return index;
   }

   //////////////////////////////////////////////////////////////////////////
   void resize(index_t new_size) override
   {
    while(size() < new_size)
     push_back();
   }

   //////////////////////////////////////////////////////////////////////////
   bool use(index_t index) override
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

    return true;
   }

   //////////////////////////////////////////////////////////////////////////
   bool free(index_t index) override
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

    return true;
   }
 };

 class Dense_Freedom_Keeper: public Freedom_Keeper
 {
  private:
   index_t used_size = 0;
   index_t free_size = 0;

  public:
   bool is_empty() const override {return used_size == 0;}
   index_t get_used_count() const override {return used_size;}
   index_t size() const override {return free_size;}

   index_t get_first_free() const override
   {
    if (used_size == free_size)
     return 1;
    else
     return used_size + 2;
   }

   index_t get_first_used() const override
   {
    if (used_size == 0)
     return 0;
    else
     return 2;
   }

   index_t get_next(index_t index) const override
   {
    if (index == 0)
     return get_first_used();

    if (index == 1)
     return get_first_free();

    const index_t result = index + 1;

    if (result == used_size + 2)
     return 0;

    if (index == free_size + 2)
     return 1;

    return result;
   }

   index_t get_previous(index_t index) const override
   {
    if (index == 0)
    {
     if (used_size == 0)
      return 0;
     else
      return used_size + 1;
    }

    if (index == 1)
    {
     if (used_size == free_size)
      return 1;
     else
      return free_size + 1;
    }

    const index_t result = index - 1;

    if (result == 1)
     return 0;
    if (result == used_size + 1)
     return 1;
    return index - 1;
   }

   bool is_free(index_t index) const override {return index - 2 >= used_size;}
   bool is_used(index_t index) const override {return index - 2 < used_size;}
   bool is_compact() const override {return true;}

   index_t get_free_record() override
   {
    if (free_size == used_size)
     ++free_size;
    return used_size + 2;
   }

   index_t push_back() override {return ++free_size + 1;}

   void resize(index_t size) override
   {
    if (free_size < size)
     free_size = size;
   }

   bool use(index_t index) override
   {
    if (index == used_size + 2 && used_size < free_size)
    {
     used_size++;
     return true;
    }
    else
     return false;
   }

   bool free(index_t index) override
   {
    if (index == used_size + 1 && index > 1)
    {
     --used_size;
     return true;
    }
    else
     return false;
   }

   bool use_vector(index_t index, index_t size) override
   {
    if (index == used_size + 2 && used_size + size <= free_size)
    {
     used_size += size;
     return true;
    }
    else
     return false;
   }

   bool append_vector(index_t size) override
   {
    if (free_size == used_size)
    {
     free_size += size;
     used_size += size;
     return true;
    }
    else
     return false;
   }
 };

 /// @ingroup joedb
 class Compact_Freedom_Keeper: public Freedom_Keeper
 {
  private:
   List_Freedom_Keeper lfk;
   Dense_Freedom_Keeper dfk;
   Freedom_Keeper *fk;

   void lose_compactness()
   {
    JOEDB_RELEASE_ASSERT(fk == &dfk);

    while (lfk.size() < dfk.size())
     lfk.push_back();

    for (index_t i = 0; i < dfk.get_used_count(); i++)
     lfk.use(i + 2);

    fk = &lfk;
   }

  public:
   Compact_Freedom_Keeper() {fk = &dfk;}
   Compact_Freedom_Keeper(const Compact_Freedom_Keeper &) = delete;
   Compact_Freedom_Keeper& operator=(const Compact_Freedom_Keeper &) = delete;

   bool is_empty() const override {return fk->is_empty();}
   index_t get_used_count() const override {return fk->get_used_count();}
   index_t size() const override {return fk->size();}
   index_t get_first_free() const override {return fk->get_first_free();}
   index_t get_first_used() const override {return fk->get_first_used();}
   index_t get_next(index_t index) const override {return fk->get_next(index);}
   index_t get_previous(index_t index) const override {return fk->get_previous(index);}
   bool is_free(index_t index) const override {return fk->is_free(index);}
   bool is_used(index_t index) const override {return fk->is_used(index);}
   bool is_compact() const override {return fk->is_compact();}

   index_t get_free_record() override {return fk->get_free_record();}
   index_t push_back() override {return fk->push_back();}
   void resize(index_t new_size) override {fk->resize(new_size);}

   bool use(index_t index) override
   {
    if (!fk->use(index))
    {
     lose_compactness();
     fk->use(index);
    }
    return true;
   }

   bool free(index_t index) override
   {
    if (!fk->free(index))
    {
     lose_compactness();
     fk->free(index);
    }
    return true;
   }

   bool use_vector(index_t index, index_t size) override
   {
    if (!fk->use_vector(index, size))
    {
     lose_compactness();
     fk->use_vector(index, size);
    }
    return true;
   }

   bool append_vector(index_t size) override
   {
    if (!fk->append_vector(size))
    {
     lose_compactness();
     fk->append_vector(size);
    }
    return true;
   }
 };
}

#endif
