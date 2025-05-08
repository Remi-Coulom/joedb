#ifndef joedb_Freedom_Keeper_declared
#define joedb_Freedom_Keeper_declared

#include "joedb/index_types.h"
#include "joedb/error/assert.h"

#include <vector>

namespace joedb
{
 class Freedom_Keeper_Constants
 {
  public:
   static constexpr Record_Id used_list{-2};
   static constexpr Record_Id free_list{-1};
 };

 class List_Data: public Freedom_Keeper_Constants
 {
  private:
   std::vector<uint8_t> is_free_v;
   std::vector<Record_Id> next_v;
   std::vector<Record_Id> previous_v;

  protected:
   uint8_t *is_free_p;
   Record_Id *next_p;
   Record_Id *previous_p;

   uint8_t &is_free_f(Record_Id id) {return is_free_p[to_underlying(id)];}
   Record_Id &next_f(Record_Id id) {return next_p[to_underlying(id)];}
   Record_Id &previous_f(Record_Id id) {return previous_p[to_underlying(id)];}

   uint8_t is_free_f(Record_Id id) const {return is_free_p[to_underlying(id)];}
   Record_Id next_f(Record_Id id) const {return next_p[to_underlying(id)];}
   Record_Id previous_f(Record_Id id) const {return previous_p[to_underlying(id)];}

   Record_Id freedom_size;

   void resize_vector(Record_Id size)
   {
    this->freedom_size = size;

    is_free_v.resize(to_underlying(size) + 2);
    next_v.resize(to_underlying(size) + 2);
    previous_v.resize(to_underlying(size) + 2);

    is_free_p = is_free_v.data() + 2;
    next_p = next_v.data() + 2;
    previous_p = previous_v.data() + 2;
   }

   List_Data() {resize_vector(Record_Id{0});}
   List_Data(const List_Data &) = delete;
   List_Data &operator=(const List_Data &) = delete;
 };

 /// @ingroup joedb
 class List_Freedom_Keeper: public List_Data
 {
  private: //////////////////////////////////////////////////////////////////
   Record_Id used_count;

  public: ///////////////////////////////////////////////////////////////////
   List_Freedom_Keeper(): used_count{0}
   {
    is_free_f(used_list) = false;
    next_f(used_list) = used_list;
    previous_f(used_list) = used_list;

    is_free_f(free_list) = true;
    next_f(free_list) = free_list;
    previous_f(free_list) = free_list;
   }

   Record_Id get_used_count() const {return used_count;}
   Record_Id get_size() const {return freedom_size;}
   size_t size() const {return size_t(freedom_size);}
   Record_Id get_first_free() const {return next_f(free_list);}
   Record_Id get_first_used() const {return next_f(used_list);}
   Record_Id get_next(const Record_Id index) const {return next_f(index);}
   Record_Id get_previous(const Record_Id index) const {return previous_f(index);}
   bool is_free(Record_Id index) const {return is_free_f(index);}
   bool is_used(Record_Id index) const
   {
    return is_not_null(index) && index < freedom_size && !is_free_f(index);
   }
   bool is_compact() const {return freedom_size == used_count;}

   //////////////////////////////////////////////////////////////////////////
   Record_Id get_free_record()
   {
    Record_Id result = next_f(free_list);
    if (result == free_list)
    {
     push_back();
     result = next_f(free_list);
    }
    return result;
   }

   //////////////////////////////////////////////////////////////////////////
   Record_Id push_back()
   {
    const Record_Id index = freedom_size;
    resize_vector(freedom_size + 1);

    is_free_f(index) = true;
    next_f(index) = next_f(free_list);
    previous_f(index) = free_list;

    previous_f(next_f(free_list)) = index;
    next_f(free_list) = index;

    return index;
   }

   //////////////////////////////////////////////////////////////////////////
   void resize(const Record_Id new_size)
   {
    while(get_size() < new_size)
     push_back();
   }

   //////////////////////////////////////////////////////////////////////////
   void use(const Record_Id index)
   {
    JOEDB_DEBUG_ASSERT(index >= Record_Id{0});
    JOEDB_DEBUG_ASSERT(index < freedom_size);
    JOEDB_DEBUG_ASSERT(is_free_f(index));

    is_free_f(index) = false;

    next_f(previous_f(index)) = next_f(index);
    previous_f(next_f(index)) = previous_f(index);

    previous_f(index) = previous_f(used_list);
    next_f(index) = used_list;

    next_f(previous_f(index)) = index;
    previous_f(used_list) = index;

    ++used_count;
   }

   //////////////////////////////////////////////////////////////////////////
   void free(Record_Id index)
   {
    JOEDB_DEBUG_ASSERT(index >= Record_Id{0});
    JOEDB_DEBUG_ASSERT(index < freedom_size);
    JOEDB_DEBUG_ASSERT(!is_free_f(index));

    is_free_f(index) = true;

    next_f(previous_f(index)) = next_f(index);
    previous_f(next_f(index)) = previous_f(index);

    next_f(index) = next_f(free_list);
    previous_f(index) = free_list;

    previous_f(next_f(free_list)) = index;
    next_f(free_list) = index;

    --used_count;
   }

   //////////////////////////////////////////////////////////////////////////
   void use_vector(Record_Id index, Record_Id size)
   {
    for (Record_Id i{0}; i < size; ++i)
     use(index + to_underlying(i));
   }
 };

 /// @ingroup joedb
 class Dense_Freedom_Keeper: public Freedom_Keeper_Constants
 {
  private:
   Record_Id used_size{0};
   Record_Id free_size{0};

  public:
   Record_Id get_used_count() const {return used_size;}
   Record_Id get_size() const {return free_size;}
   size_t size() const {return size_t(free_size);}

   Record_Id get_first_free() const
   {
    if (used_size == free_size)
     return free_list;
    else
     return used_size;
   }

   Record_Id get_first_used() const
   {
    if (to_underlying(used_size) == 0)
     return used_list;
    else
     return Record_Id{0};
   }

   Record_Id get_next(Record_Id index) const
   {
    if (index == used_list)
     return get_first_used();

    if (index == free_list)
     return get_first_free();

    const Record_Id result = index + 1;

    if (result == used_size)
     return used_list;

    if (result == free_size)
     return free_list;

    return result;
   }

   Record_Id get_previous(Record_Id index) const
   {
    if (index == used_list)
    {
     if (used_size == Record_Id{0})
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

    const Record_Id result = index - 1;

    if (result == Record_Id{-1})
     return used_list;

    if (result == used_size - 1)
     return free_list;

    return result;
   }

   bool is_free(Record_Id index) const {return index >= used_size;}
   bool is_used(Record_Id index) const {return index < used_size;}
   bool is_compact() const {return true;}

   Record_Id get_free_record()
   {
    if (free_size == used_size)
     ++free_size;
    return used_size;
   }

   Record_Id push_back() {return ++free_size - 1;}

   void resize(Record_Id size)
   {
    if (free_size < size)
     free_size = size;
   }

   bool use(Record_Id index)
   {
    if (index == used_size && used_size < free_size)
    {
     ++used_size;
     return true;
    }
    else
     return false;
   }

   bool free(Record_Id index)
   {
    if (index == used_size - 1 && index >= Record_Id{0})
    {
     --used_size;
     return true;
    }
    else
     return false;
   }

   bool use_vector(Record_Id index, Record_Id size)
   {
    if (index == used_size && used_size + to_underlying(size) <= free_size)
    {
     used_size = used_size + to_underlying(size);
     return true;
    }
    else
     return false;
   }
 };

 /// @ingroup joedb
 class Freedom_Keeper
 {
  private:
   List_Freedom_Keeper lfk;
   Dense_Freedom_Keeper dfk;

   bool dense = true;

   void lose_compactness()
   {
    JOEDB_RELEASE_ASSERT(dense);

    while (lfk.get_size() < dfk.get_size())
     lfk.push_back();

    for (Record_Id i{0}; i < dfk.get_used_count(); ++i)
     lfk.use(i);

    dense = false;
   }

  public:
#define SWITCH(f) (dense ? dfk.f : lfk.f)
   Record_Id get_used_count() const {return SWITCH(get_used_count());}
   Record_Id get_size() const {return SWITCH(get_size());}
   size_t size() const {return SWITCH(size());}

   Record_Id get_first_free() const {return SWITCH(get_first_free());}
   Record_Id get_first_used() const {return SWITCH(get_first_used());}
   Record_Id get_next(Record_Id index) const {return SWITCH(get_next(index));}
   Record_Id get_previous(Record_Id index) const {return SWITCH(get_previous(index));}
   bool is_free(Record_Id index) const {return SWITCH(is_free(index));}
   bool is_used(Record_Id index) const {return SWITCH(is_used(index));}
   bool is_compact() const {return SWITCH(is_compact());}

   Record_Id get_free_record() {return SWITCH(get_free_record());}
   Record_Id push_back() {return SWITCH(push_back());}
   void resize(Record_Id new_size) {SWITCH(resize(new_size));}
   void resize(size_t new_size) {resize(Record_Id(new_size));}
#undef SWITCH

   void use(Record_Id index)
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

   void free(Record_Id index)
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

   void use_vector(Record_Id index, Record_Id size)
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
