#ifndef joedb_Readonly_Journal_declared
#define joedb_Readonly_Journal_declared

#include "Writeable.h"

namespace joedb
{
 class Generic_File;

 class Readonly_Journal
 {
  public:
   Readonly_Journal(Generic_File &file, bool ignore_errors = false);

   int64_t get_checkpoint_position() const {return checkpoint_position;}
   void replay_log(Writeable &writeable);
   void rewind();
   void seek(int64_t position);
   void play_until(Writeable &writeable, int64_t end);
   void play_until_checkpoint(Writeable &writeable)
   {
    play_until(writeable, checkpoint_position);
   }

   static const uint32_t version_number;
   static const uint32_t compatible_version;
   static const int64_t header_size;

  protected:
   Generic_File &file;
   unsigned checkpoint_index;
   int64_t checkpoint_position;

   Table_Id table_of_last_operation;
   Record_Id record_of_last_operation;
   Field_Id field_of_last_update;

   Type read_type();
   std::string safe_read_string();

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
    #include "TYPE_MACRO.h"
    #undef TYPE_MACRO
   };
 };
}

#endif
