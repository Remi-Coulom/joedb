#ifndef joedb_Writable_declared
#define joedb_Writable_declared

#include "joedb/Type.h"

#include <string>

namespace joedb
{
 /// Superclass with all joedb journal event listeners as virtual functions
 ///
 /// insert_vector and delete_vector are pure virtual because they do not
 /// check size limits, so they can be dangerously slow.
 ///
 /// @ingroup joedb
 class Writable
 {
  public:
   virtual int64_t get_position() const {return 0;}
   virtual void start_writing(int64_t position) {}
   virtual void end_writing(int64_t position) {}

   virtual void soft_checkpoint() {}
   virtual void hard_checkpoint() {soft_checkpoint();}

   virtual void create_table(const std::string &name) {}
   virtual void drop_table(Table_Id table_id) {}
   virtual void rename_table(Table_Id table_id, const std::string &name) {}

   virtual void add_field
   (
    Table_Id table_id,
    const std::string &name,
    Type type
   ) {}
   virtual void drop_field(Table_Id table_id, Field_Id field_id) {}
   virtual void rename_field
   (
    Table_Id table_id,
    Field_Id field_id,
    const std::string &name
   ) {}

   virtual void custom(const std::string &name) {}
   virtual void comment(const std::string &comment) {}
   virtual void timestamp(int64_t timestamp) {}
   virtual void valid_data() {}
   virtual void flush() {}

   virtual void insert_into(Table_Id table_id, Record_Id record_id) {}
   virtual void delete_from(Table_Id table_id, Record_Id record_id) {}

   virtual void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) = 0;

   virtual void delete_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) = 0;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   virtual void update_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    return_type value\
   );
   #include "joedb/TYPE_MACRO.h"

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   virtual void update_vector_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    size_t size,\
    const type *value\
   );\
   virtual type *get_own_##type_id##_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    size_t &capacity\
   )\
   {\
    capacity = 0;\
    return nullptr;\
   }\
   const type *get_own_##type_id##_const_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    size_t &capacity\
   ) const\
   {\
    return (const_cast<Writable *>(this))->get_own_##type_id##_storage(table_id, record_id, field_id, capacity);\
   }
   #include "joedb/TYPE_MACRO.h"

   virtual void on_blob(Blob blob) {}
   virtual bool wants_blob_data() const {return false;}
   virtual Blob write_blob(const std::string &data) {return Blob();}

   virtual ~Writable() = default;
 };

 /// Writable with empty insert_vector and delete_vector
 ///
 /// @ingroup joedb
 class Dummy_Writable: public Writable
 {
  public:
   void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) override
   {
   }

   void delete_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) override
   {
   }
 };

 /// Writable with looping insert_vector and delete_vector
 ///
 /// @ingroup joedb
 class Loop_Writable: public Writable
 {
  public:
   void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) override
   {
    Writable::insert_vector(table_id, record_id, size);
   }

   void delete_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) override
   {
    Writable::delete_vector(table_id, record_id, size);
   }
 };
}

#endif
