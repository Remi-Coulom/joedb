#ifndef joedb_Freedom_Keeper_declared
#define joedb_Freedom_Keeper_declared

#include <cstddef>
#include <vector>
#include <cassert>

namespace joedb
{
 class EmptyRecord
 {
  private:
   bool f;

  public:
   EmptyRecord() {}
   EmptyRecord(bool f): f(f) {}
   void set_free(bool new_f) {f = new_f;}
   bool is_free() const {return f;}
 };

 template<typename T = EmptyRecord>
 class Freedom_Keeper
 {
  private: //////////////////////////////////////////////////////////////////
   struct Record
   {
    T data;
    size_t next;
    size_t previous;
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

   T &get_record(size_t index) {return records[index].data;}
   const T &get_record(size_t index) const {return records[index].data;}

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
    records.push_back({true, records[free_list].next, 1});

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
    assert(index > 1);
    assert(index < records.size());
    assert(records[index].data.is_free());

    Record &record = records[index];
    record.data.set_free(false);

    records[record.previous].next = record.next;
    records[record.next].previous = record.previous;

    record.next = records[used_list].next;
    record.previous = 0;

    records[records[used_list].next].previous = index;
    records[used_list].next = index;

    used_count++;
   }

   //////////////////////////////////////////////////////////////////////////
   void free(size_t index)
   {
    assert(index > 1);
    assert(index < records.size());
    assert(!records[index].data.is_free());

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
}

#endif
