#ifndef joedb_Compiler_Options_declared
#define joedb_Compiler_Options_declared

#include "joedb/interpreter/Database.h"

#include <string>
#include <vector>

namespace joedb
{
 ///////////////////////////////////////////////////////////////////////////
 class Compiler_Options
 ///////////////////////////////////////////////////////////////////////////
 {
  public:
   struct Index
   {
    bool unique;
    std::string name;
    Table_Id table_id;
    std::vector<Field_Id> field_ids;
   };

   struct Table_Options
   {
    bool null_initialization;
   };

  private:
   const Database &db;
   const std::vector<std::string> &custom_names;

   std::vector<std::string> name_space;
   std::vector<Index> indices;
   std::map<Table_Id, Table_Options> table_options;

   bool generate_c_wrapper;
   bool generate_js_wrapper;

  public:
   Compiler_Options
   (
    const Database &db,
    const std::vector<std::string> &custom_names
   ):
    db(db),
    custom_names(custom_names),
    generate_c_wrapper(false)
   {
    for (auto table: db.get_tables())
    {
     table_options[table.first].null_initialization = false;
    }
   }

   void set_name_space(std::vector<std::string> v)
   {
    name_space = std::move(v);
   }
   void set_table_null_initialization
   (
    Table_Id table_id,
    bool null_initialization
   )
   {
    table_options[table_id].null_initialization = null_initialization;
   }
   void add_index(Index index)
   {
    indices.emplace_back(std::move(index));
   }
   void set_generate_c_wrapper(bool value) {generate_c_wrapper = value;}
   void set_generate_js_wrapper(bool value) {generate_js_wrapper = value;}

   const Database &get_db() const {return db;}
   const std::vector<std::string> &get_custom_names() const
   {
    return custom_names;
   }
   const std::vector<std::string> &get_name_space() const
   {
    return name_space;
   }
   const std::vector<Index> &get_indices() const {return indices;}
   const Table_Options &get_table_options(Table_Id table_id) const
   {
    return table_options.find(table_id)->second;
   }
   bool get_generate_c_wrapper() const {return generate_c_wrapper;}
   bool get_generate_js_wrapper() const {return generate_js_wrapper;}
 };
}

#endif
