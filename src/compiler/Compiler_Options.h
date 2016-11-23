#ifndef joedb_Compiler_Options_declared
#define joedb_Compiler_Options_declared

#include <string>
#include <vector>

#include "Database.h"

namespace joedb
{
 class Compiler_Options
 {
  public:
   struct Index
   {
    bool unique;
    std::string name;
    table_id_t table_id;
    std::vector<field_id_t> field_ids;
   };

   enum Table_Storage
   {
    freedom_keeper,
    vector
   };

   struct Table_Options
   {
    Table_Storage storage;
   };

  private:
   const Database &db;

   std::string namespace_name;
   std::vector<Index> indices;
   std::vector<Table_Options> table_options;

   bool generate_c_wrapper;
   bool generate_js_wrapper;

  public:
   Compiler_Options(const Database &db):
    db(db),
    table_options(db.get_current_table_id(), {freedom_keeper}),
    generate_c_wrapper(false)
   {
   }

   void set_namespace_name(const std::string &s) {namespace_name = s;}
   void set_table_storage(table_id_t table_id, Table_Storage storage)
   {
    table_options[table_id - 1].storage = storage;
   }
   void add_index(const Index &index) {indices.push_back(index);}
   void set_generate_c_wrapper(bool value) {generate_c_wrapper = value;}
   void set_generate_js_wrapper(bool value) {generate_js_wrapper = value;}

   const Database &get_db() const {return db;}
   const std::string &get_namespace_name() const {return namespace_name;}
   const std::vector<Index> &get_indices() const {return indices;}
   const Table_Options &get_table_options(table_id_t table_id) const
   {
    return table_options[table_id - 1];
   }
   bool get_generate_c_wrapper() const {return generate_c_wrapper;}
   bool get_generate_js_wrapper() const {return generate_js_wrapper;}
 };
}

#endif
