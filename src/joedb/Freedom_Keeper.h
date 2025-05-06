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

   virtual ~Freedom_Keeper() = default;

   static constexpr index_t used_list = -2;
   static constexpr index_t free_list = -1;
 };

 class List_Data
 {
  private:
   std::vector<uint8_t> is_free_v;
   std::vector<index_t> next_v;
   std::vector<index_t> previous_v;

  protected:
   uint8_t *is_free_p;
   index_t *next_p;
   index_t *previous_p;

   index_t freedom_size;

   void resize_vector(index_t size)
   {
    this->freedom_size = size;

    is_free_v.resize(freedom_size + 2);
    next_v.resize(freedom_size + 2);
    previous_v.resize(freedom_size + 2);

    is_free_p = is_free_v.data() + 2;
    next_p = next_v.data() + 2;
    previous_p = previous_v.data() + 2;
   }

   List_Data() {resize_vector(0);}
   List_Data(const List_Data &) = delete;
   List_Data &operator=(const List_Data &) = delete;
 };

 /// @ingroup joedb
 class List_Freedom_Keeper: public List_Data, public Freedom_Keeper
 {
  private: //////////////////////////////////////////////////////////////////
   index_t used_count;

  public: ///////////////////////////////////////////////////////////////////
   List_Freedom_Keeper(): used_count(0)
   {
    is_free_p[used_list] = false;
    next_p[used_list] = used_list;
    previous_p[used_list] = used_list;

    is_free_p[free_list] = true;
    next_p[free_list] = free_list;
    previous_p[free_list] = free_list;
   }

   index_t get_used_count() const override {return used_count;}
   index_t size() const override {return freedom_size;}
   index_t get_first_free() const override {return next_p[free_list];}
   index_t get_first_used() const override {return next_p[used_list];}
   index_t get_next(const index_t index) const override {return next_p[index];}
   index_t get_previous(const index_t index) const override {return previous_p[index];}
   bool is_free(index_t index) const override {return is_free_p[index];}
   bool is_used(index_t index) const override
   {
    return index >= 0 && index < freedom_size && !is_free_p[index];
   }
   bool is_compact() const override {return freedom_size == used_count;}

   //////////////////////////////////////////////////////////////////////////
   index_t get_free_record() override
   {
    index_t result = next_p[free_list];
    if (result == free_list)
    {
     push_back();
     result = next_p[free_list];
    }
    return result;
   }

   //////////////////////////////////////////////////////////////////////////
   index_t push_back() override
   {
    const index_t index = freedom_size;
    resize_vector(freedom_size + 1);

    is_free_p[index] = true;
    next_p[index] = next_p[free_list];
    previous_p[index] = free_list;

    previous_p[next_p[free_list]] = index;
    next_p[free_list] = index;

    return index;
   }

   //////////////////////////////////////////////////////////////////////////
   void resize(const index_t new_size) override
   {
    while(size() < new_size)
     push_back();
   }

   //////////////////////////////////////////////////////////////////////////
   bool use(const index_t index) override
   {
    JOEDB_DEBUG_ASSERT(index >= 0);
    JOEDB_DEBUG_ASSERT(index < freedom_size);
    JOEDB_DEBUG_ASSERT(is_free_p[index]);

    is_free_p[index] = false;

    next_p[previous_p[index]] = next_p[index];
    previous_p[next_p[index]] = previous_p[index];

    previous_p[index] = previous_p[used_list];
    next_p[index] = used_list;

    next_p[previous_p[index]] = index;
    previous_p[used_list] = index;

    used_count++;

    return true;
   }

   //////////////////////////////////////////////////////////////////////////
   bool free(index_t index) override
   {
    JOEDB_DEBUG_ASSERT(index >= 0);
    JOEDB_DEBUG_ASSERT(index < freedom_size);
    JOEDB_DEBUG_ASSERT(!is_free_p[index]);

    is_free_p[index] = true;

    next_p[previous_p[index]] = next_p[index];
    previous_p[next_p[index]] = previous_p[index];

    next_p[index] = next_p[free_list];
    previous_p[index] = free_list;

    previous_p[next_p[free_list]] = index;
    next_p[free_list] = index;

    used_count--;

    return true;
   }
 };

 /// @ingroup joedb
 class Dense_Freedom_Keeper: public Freedom_Keeper
 {
  private:
   index_t used_size = 0;
   index_t free_size = 0;

  public:
   index_t get_used_count() const override {return used_size;}
   index_t size() const override {return free_size;}

   index_t get_first_free() const override
   {
    if (used_size == free_size)
     return free_list;
    else
     return used_size;
   }

   index_t get_first_used() const override
   {
    if (used_size == 0)
     return used_list;
    else
     return 0;
   }

   index_t get_next(index_t index) const override
   {
    if (index == used_list)
     return get_first_used();

    if (index == free_list)
     return get_first_free();

    const index_t result = index + 1;

    if (result == used_size)
     return used_list;

    if (result == free_size)
     return free_list;

    return result;
   }

   index_t get_previous(index_t index) const override
   {
    if (index == used_list)
    {
     if (used_size == 0)
      return used_list;
     else
      return used_size - 1;
    }

    if (index == free_list)
    {
     if (used_size == free_size)
      return free_list;
     else
      return free_size - 1;
    }

    const index_t result = index - 1;

    if (result == -1)
     return used_list;

    if (result == used_size - 1)
     return free_list;

    return result;
   }

   bool is_free(index_t index) const override {return index >= used_size;}
   bool is_used(index_t index) const override {return index < used_size;}
   bool is_compact() const override {return true;}

   index_t get_free_record() override
   {
    if (free_size == used_size)
     ++free_size;
    return used_size;
   }

   index_t push_back() override {return free_size++;}

   void resize(index_t size) override
   {
    if (free_size < size)
     free_size = size;
   }

   bool use(index_t index) override
   {
    if (index == used_size && used_size < free_size)
    {
     used_size++;
     return true;
    }
    else
     return false;
   }

   bool free(index_t index) override
   {
    if (index == used_size - 1 && index >= 0)
    {
     --used_size;
     return true;
    }
    else
     return false;
   }

   bool use_vector(index_t index, index_t size) override
   {
    if (index == used_size && used_size + size <= free_size)
    {
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
     lfk.use(i);

    fk = &lfk;
   }

  public:
   Compact_Freedom_Keeper() {fk = &dfk;}
   Compact_Freedom_Keeper(const Compact_Freedom_Keeper &) = delete;
   Compact_Freedom_Keeper& operator=(const Compact_Freedom_Keeper &) = delete;

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
 };
}

#endif
