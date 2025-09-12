#ifndef joedb_Readonly_Journal_declared
#define joedb_Readonly_Journal_declared

#include "joedb/Writable.h"
#include "joedb/journal/Async_Reader.h"
#include "joedb/journal/Journal_Construction_Lock.h"
#include "joedb/journal/Header.h"

#include <array>

namespace joedb
{
 class Writable_Journal;

 /// @ingroup journal
 class Readonly_Journal
 {
  friend class Writable_Journal;
  friend class Journal_Hasher;

  private:
   void read_checkpoint(const std::array<int64_t, 4> &pos, int64_t file_size);
   void pull_without_locking();

   #define TYPE_MACRO(cpp_type, return_type, type_id, read_method, W)\
   void perform_update_##type_id(Writable &writable);
   #include "joedb/TYPE_MACRO.h"

  protected:
   Buffered_File &file;
   int hard_index;
   int soft_index;
   int64_t checkpoint_position;
   int64_t hard_checkpoint_position;

   Table_Id table_of_last_operation;
   Record_Id record_of_last_operation;
   Field_Id field_of_last_update;

   void reset_context();

   Type read_type();
   std::string safe_read_string();

   #define TYPE_MACRO(cpp_type, return_type, type_id, read_method, W)\
   void read_vector_of_##type_id(cpp_type *data, size_t size);
   #include "joedb/TYPE_MACRO.h"

   enum class operation_t: uint8_t
   {
    create_table  = 0x01,
    drop_table    = 0x02,
    add_field     = 0x03,
    drop_field    = 0x04,
    insert_into   = 0x05,
    delete_from   = 0x06,
    update        = 0x07, // deprecated
    append        = 0x08,
    update_last   = 0x09, // deprecated
    comment       = 0x0a,
    timestamp     = 0x0b,
    rename_table  = 0x0c,
    rename_field  = 0x0d,
    valid_data    = 0x0e,
    insert_vector = 0x0f,
    custom        = 0x10,
    update_vector = 0x11, // deprecated
    update_next   = 0x12, // deprecated
    blob          = 0x13,
    delete_vector = 0x14,
    updates       = 0x80,
    #define TYPE_MACRO(t, rt, type_id, r, w)\
    update_##type_id,\
    update_last_##type_id,\
    update_next_##type_id,\
    update_vector_##type_id,
    #include "joedb/TYPE_MACRO.h"
   };

  public:
   explicit Readonly_Journal(Journal_Construction_Lock &lock);

   explicit Readonly_Journal(Journal_Construction_Lock &&lock):
    Readonly_Journal(lock)
   {
   }

   explicit Readonly_Journal(Buffered_File &file):
    Readonly_Journal(Journal_Construction_Lock(file))
   {
   }

   int64_t get_position() const {return file.get_position();}
   int64_t get_checkpoint() const {return checkpoint_position;}
   int64_t get_hard_checkpoint() const {return hard_checkpoint_position;}
   bool is_empty() const {return file.get_size() == Header::ssize;}
   bool is_shared() const {return file.is_shared();}
   int64_t pull();
   const Buffered_File &get_file() const {return file;}
   void replay_log(Writable &writable);
   void replay_with_checkpoint_comments(Writable &writable);
   void rewind();
   void one_step(Writable &writable);
   void play_until(Writable &writable, int64_t end);
   void raw_play_until(Writable &writable, int64_t end);
   void play_until_checkpoint(Writable &writable)
   {
    play_until(writable, checkpoint_position);
   }
   void raw_play_until_checkpoint(Writable &writable)
   {
    raw_play_until(writable, checkpoint_position);
   }
   void skip_directly_to(int64_t position)
   {
    file.set_position(position);
   }
   bool equal_to(Readonly_Journal &journal, int64_t from, int64_t until) const
   {
    return file.equal_to(journal.file, from, until);
   }

   Async_Reader get_async_tail_reader(int64_t start_position) const
   {
    return Async_Reader(file, start_position, get_checkpoint());
   }

   Async_Reader get_async_reader(int64_t start_position, int64_t until_position) const
   {
    return Async_Reader
    (
     file,
     start_position,
     until_position
    );
   }

   static constexpr uint32_t format_version = 5;
 };
}

#endif
