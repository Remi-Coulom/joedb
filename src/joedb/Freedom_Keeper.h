#ifndef joedb_Freedom_Keeper_declared
#define joedb_Freedom_Keeper_declared

#include "joedb/index_types.h"
#include "joedb/error/assert.h"

#include <vector>

namespace joedb
{
 class Freedom_Keeper
 {
  public:
   static constexpr index_t used_list = -2;
   static constexpr index_t free_list = -1;
 };

 class List_Data: public Freedom_Keeper
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
 class List_Freedom_Keeper: public List_Data
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

   index_t get_used_count() const {return used_count;}
   index_t size() const {return freedom_size;}
   index_t get_first_free() const {return next_p[free_list];}
   index_t get_first_used() const {return next_p[used_list];}
   index_t get_next(const index_t index) const {return next_p[index];}
   index_t get_previous(const index_t index) const {return previous_p[index];}
   bool is_free(index_t index) const {return is_free_p[index];}
   bool is_used(index_t index) const
   {
    return index >= 0 && index < freedom_size && !is_free_p[index];
   }
   bool is_compact() const {return freedom_size == used_count;}

   //////////////////////////////////////////////////////////////////////////
   index_t get_free_record()
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
   index_t push_back()
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
   void resize(const index_t new_size)
   {
    while(size() < new_size)
     push_back();
   }

   //////////////////////////////////////////////////////////////////////////
   void use(const index_t index)
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
   }

   //////////////////////////////////////////////////////////////////////////
   void free(index_t index)
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
   }

   //////////////////////////////////////////////////////////////////////////
   void use_vector(index_t index, index_t size)
   {
    for (index_t i = 0; i < size; i++)
     use(index + i);
   }
 };

 /// @ingroup joedb
 class Dense_Freedom_Keeper: public Freedom_Keeper
 {
  private:
   index_t used_size = 0;
   index_t free_size = 0;

  public:
   index_t get_used_count() const {return used_size;}
   index_t size() const {return free_size;}

   index_t get_first_free() const
   {
    if (used_size == free_size)
     return free_list;
    else
     return used_size;
   }

   index_t get_first_used() const
   {
    if (used_size == 0)
     return used_list;
    else
     return 0;
   }

   index_t get_next(index_t index) const
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

   index_t get_previous(index_t index) const
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

   bool is_free(index_t index) const {return index >= used_size;}
   bool is_used(index_t index) const {return index < used_size;}
   bool is_compact() const {return true;}

   index_t get_free_record()
   {
    if (free_size == used_size)
     ++free_size;
    return used_size;
   }

   index_t push_back() {return free_size++;}

   void resize(index_t size)
   {
    if (free_size < size)
     free_size = size;
   }

   bool use(index_t index)
   {
    if (index == used_size && used_size < free_size)
    {
     used_size++;
     return true;
    }
    else
     return false;
   }

   bool free(index_t index)
   {
    if (index == used_size - 1 && index >= 0)
    {
     --used_size;
     return true;
    }
    else
     return false;
   }

   bool use_vector(index_t index, index_t size)
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

   bool dense = true;

   void lose_compactness()
   {
    JOEDB_RELEASE_ASSERT(dense);

    while (lfk.size() < dfk.size())
     lfk.push_back();

    for (index_t i = 0; i < dfk.get_used_count(); i++)
     lfk.use(i);

    dense = false;
   }

  public:
#define SWITCH(f) (dense ? dfk.f : lfk.f)
   index_t get_used_count() const {return SWITCH(get_used_count());}
   index_t size() const {return SWITCH(size());}

   index_t get_first_free() const {return SWITCH(get_first_free());}
   index_t get_first_used() const {return SWITCH(get_first_used());}
   index_t get_next(index_t index) const {return SWITCH(get_next(index));}
   index_t get_previous(index_t index) const {return SWITCH(get_previous(index));}
   bool is_free(index_t index) const {return SWITCH(is_free(index));}
   bool is_used(index_t index) const {return SWITCH(is_used(index));}
   bool is_compact() const {return SWITCH(is_compact());}

   index_t get_free_record() {return SWITCH(get_free_record());}
   index_t push_back() {return SWITCH(push_back());}
   void resize(index_t new_size) {SWITCH(resize(new_size));}
#undef SWITCH

   void use(index_t index)
   {
    if (dense)
    {
     if (!dfk.use(index))
     {
      lose_compactness();
      lfk.use(index);
     }
    }
    else
     lfk.use(index);
   }

   void free(index_t index)
   {
    if (dense)
    {
     if (!dfk.free(index))
     {
      lose_compactness();
      lfk.free(index);
     }
    }
    else
     lfk.free(index);
   }

   void use_vector(index_t index, index_t size)
   {
    if (dense)
    {
     if (!dfk.use_vector(index, size))
     {
      lose_compactness();
      lfk.use_vector(index, size);
     }
    }
    else
     lfk.use_vector(index, size);
   }
 };
}

#endif
