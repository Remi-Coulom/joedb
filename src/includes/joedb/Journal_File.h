#ifndef joedb_Journal_File_declared
#define joedb_Journal_File_declared

#include "Listener.h"
#include "Database.h"

namespace joedb
{
 class Generic_File;

 class Journal_File: public Listener
 {
  public:
   Journal_File(Generic_File &file);

   enum class state_t
   {
    no_error,
    bad_file,
    unsupported_version,
    bad_format,
    crash_check
   };

   state_t get_state() const {return state;}
   uint64_t ahead_of_checkpoint() const;
   void checkpoint(int commit_level);
   void replay_log(Listener &listener);
   void rewind();
   void play_until(Listener &listener, uint64_t end);

   void after_create_table(const std::string &name) override;
   void after_drop_table(table_id_t table_id) override;
   void after_rename_table(table_id_t table_id,
                           const std::string &name) override;
   void after_add_field(table_id_t table_id,
                        const std::string &name,
                        Type type) override;
   void after_drop_field(table_id_t table_id,
                         field_id_t field_id) override;
   void after_rename_field(table_id_t table_id,
                           field_id_t field_id,
                           const std::string &name) override;
   void after_custom(const std::string &name) override;
   void after_comment(const std::string &comment) override;
   void after_timestamp(int64_t timestamp) override;
   void after_insert(table_id_t table_id, record_id_t record_id) override;
   void after_insert_vector(table_id_t table_id,
                            record_id_t record_id,
                            record_id_t size) override;
   void after_delete(table_id_t table_id, record_id_t record_id) override;

   #define TYPE_MACRO(type, return_type, type_id, read_method, write_method)\
   void after_update_##type_id(table_id_t table_id,\
                               record_id_t record_id,\
                               field_id_t field_id,\
                               return_type value) override;\
   void after_update_vector_##type_id(table_id_t table_id,\
                                      record_id_t record_id,\
                                      field_id_t field_id,\
                                      record_id_t size,\
                                      const type *value) override;
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO

   ~Journal_File() override;

   static const uint32_t version_number;
   static const uint64_t header_size;

  private:
   Generic_File &file;
   unsigned checkpoint_index;
   uint64_t checkpoint_position;
   int current_commit_level;
   state_t state;

   Database db_schema;
   table_id_t table_of_last_operation;
   record_id_t record_of_last_operation;
   field_id_t field_of_last_update;
   Type::type_id_t type_of_last_update;

   Type read_type();

   void read_update(table_id_t table_id, record_id_t record_id);

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
