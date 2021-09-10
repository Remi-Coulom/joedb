#ifndef joedb_Readonly_Journal_declared
#define joedb_Readonly_Journal_declared

#include "joedb/Writable.h"
#include "joedb/journal/Async_Reader.h"

#include <vector>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Journal
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   void read_checkpoint();

  protected:
   Generic_File &file;
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
    end_of_file   = 0x00,
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
    updates       = 0x80,
    #define TYPE_MACRO(t, rt, type_id, r, w)\
    update_##type_id,\
    update_last_##type_id,\
    update_next_##type_id,\
    update_vector_##type_id,
    #include "joedb/TYPE_MACRO.h"
   };

  public:
   Readonly_Journal(Generic_File &file, bool ignore_errors = false);

   bool at_end_of_file() const;
   int64_t get_position() const {return file.get_position();}
   int64_t get_checkpoint_position() const {return checkpoint_position;}
   bool is_empty() const {return file.get_size() == header_size;}
   bool is_same_file(const Generic_File &other_file) const
   {
    return &file == &other_file;
   }

   void refresh_checkpoint();
   void replay_log(Writable &writable);
   void rewind();
   void seek(int64_t position);
   void one_step(Writable &writable);
   void play_until(Writable &writable, int64_t end);
   void play_until_checkpoint(Writable &writable)
   {
    play_until(writable, checkpoint_position);
    writable.checkpoint(Commit_Level::no_commit);
   }

   Async_Reader get_tail_reader(int64_t start_position) const
   {
    return Async_Reader(file, start_position, get_checkpoint_position());
   }

   std::vector<char> get_raw_tail(int64_t starting_position) const;

   SHA_256::Hash get_hash(int64_t checkpoint)
   {
    return file.get_hash(header_size, checkpoint - header_size);
   }

   static constexpr uint32_t version_number = 0x00000004;
   static constexpr uint32_t compatible_version = 0x00000004;
   static constexpr int64_t header_size = 41;
 };
}

#endif
