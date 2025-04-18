#ifndef joedb_Readonly_Journal_declared
#define joedb_Readonly_Journal_declared

#include "joedb/Writable.h"
#include "joedb/journal/Async_Reader.h"
#include "joedb/journal/Journal_Construction_Lock.h"

#include <array>

namespace joedb
{
 class Writable_Journal;

 /// @ingroup journal
 class Readonly_Journal
 {
  friend class Writable_Journal;
  friend class Journal_Hasher;

  public:
   enum class Check
   {
    joedb = 1,
    version = 2,
    big_size = 4,
    small_size = 8,
    set_checkpoint = 16,
    checkpoint_mismatch = 32,
    all = joedb | version | big_size | small_size | checkpoint_mismatch,
    readonly = joedb | version | small_size,
    overwrite = all & ~big_size & ~checkpoint_mismatch,
    none = set_checkpoint
   };

   static bool check_flag(Check check, Check flag)
   {
    return static_cast<int>(check) & static_cast<int>(flag);
   }

  private:
   void read_checkpoint(const std::array<int64_t, 4> &pos);
   void pull_without_locking();

   #define TYPE_MACRO(cpp_type, return_type, type_id, read_method, W)\
   void perform_update_##type_id(Writable &writable);
   #include "joedb/TYPE_MACRO.h"

  protected:
   Buffered_File &file;
   uint32_t file_version;
   unsigned checkpoint_index;
   int64_t checkpoint_position;

   Table_Id table_of_last_operation;
   Record_Id record_of_last_operation;
   Field_Id field_of_last_update;

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
    updates       = 0x80,
    #define TYPE_MACRO(t, rt, type_id, r, w)\
    update_##type_id,\
    update_last_##type_id,\
    update_next_##type_id,\
    update_vector_##type_id,
    #include "joedb/TYPE_MACRO.h"
   };

  public:
   explicit Readonly_Journal(Journal_Construction_Lock &lock, Check check);

   explicit Readonly_Journal(Journal_Construction_Lock &&lock, Check check):
    Readonly_Journal(lock, check)
   {
   }

   explicit Readonly_Journal
   (
    Buffered_File &file,
    Check check = Check::readonly
   ):
    Readonly_Journal(Journal_Construction_Lock(file), check)
   {
   }

   uint32_t get_file_version() const {return file_version;}
   bool at_end_of_file() const;
   int64_t get_position() const {return file.get_position();}
   int64_t get_checkpoint_position() const {return checkpoint_position;}
   bool is_empty() const {return file.get_size() == header_size;}
   bool is_shared() const {return file.is_shared();}
   void pull();
   const Buffered_File &get_file() const {return file;}
   void replay_log(Writable &writable);
   void replay_with_checkpoint_comments(Writable &writable);
   void rewind();
   void set_position(int64_t position);
   void one_step(Writable &writable);
   void play_until(Writable &writable, int64_t end);
   void play_until_checkpoint(Writable &writable)
   {
    play_until(writable, checkpoint_position);
    writable.default_checkpoint();
   }
   void seek_to_checkpoint() {set_position(checkpoint_position);}

   Async_Reader get_async_tail_reader(int64_t start_position) const
   {
    return Async_Reader(file, start_position, get_checkpoint_position());
   }

   Async_Reader get_async_reader(int64_t start_position, int64_t until_position) const
   {
    return Async_Reader
    (
     file,
     start_position,
     std::min(until_position, get_checkpoint_position())
    );
   }

   Async_Reader get_async_blob_reader(Blob blob) const
   {
    return Async_Reader(file, blob);
   }

   static constexpr uint32_t version_number = 0x00000004;
   static constexpr uint32_t compatible_version = 0x00000004;
   static constexpr int64_t header_size = 41;
   static constexpr int64_t checkpoint_offset = 5 + 4;
   static constexpr bool is_second_checkpoint_copy(int64_t offset)
   {
    return offset == 17 || offset == 33;
   }

   virtual Writable_Journal *get_writable_journal() {return nullptr;}
   virtual ~Readonly_Journal() = default;
 };
}

#endif
