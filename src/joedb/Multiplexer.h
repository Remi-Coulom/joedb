#ifndef joedb_Multiplexer_declared
#define joedb_Multiplexer_declared

#include <vector>
#include <initializer_list>
#include <functional>

#include "Writable.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Multiplexer: public virtual Writable
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::vector<std::reference_wrapper<Writable>> writables;

  public:
   Multiplexer() {}
   Multiplexer(std::initializer_list<std::reference_wrapper<Writable>>);
   void add_writable(Writable &writable);

   void create_table(const std::string &name) final override;
   void drop_table(Table_Id table_id) final override;
   void rename_table(Table_Id table_id, const std::string &name) final override;
   void add_field
   (
    Table_Id table_id,
    const std::string &name,
    Type type
   ) final override;
   void drop_field(Table_Id table_id, Field_Id field_id) final override;
   void rename_field
   (
    Table_Id table_id,
    Field_Id field_id,
    const std::string &name
   ) final override;
   void custom(const std::string &name) final override;
   void comment(const std::string &comment) final override;
   void timestamp(int64_t timestamp) final override;
   void valid_data() final override;
   void checkpoint(Commit_Level commit_level) final override;
   void insert_into(Table_Id table_id, Record_Id record_id) final override;
   void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    Record_Id size
   ) final override;
   void delete_from(Table_Id table_id, Record_Id record_id) final override;
   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    return_type value\
   ) final override;\
   void update_vector_##type_id\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    Record_Id size,\
    const type *value\
   ) final override;\
   type *get_own_##type_id##_storage\
   (\
    Table_Id table_id,\
    Record_Id record_id,\
    Field_Id field_id,\
    Record_Id &capacity\
   ) final override;
   #include "joedb/TYPE_MACRO.h"

   ~Multiplexer();
 };
}

#endif
