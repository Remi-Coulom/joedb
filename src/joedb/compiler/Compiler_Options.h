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
    bool single_row = false;
   };

  private:
   const Database &db;
   const std::vector<std::string> &custom_names;

   std::vector<std::string> name_space;
   std::vector<Index> indices;
   std::map<Table_Id, Table_Options> table_options;

  public:
   Compiler_Options
   (
    const Database &db,
    const std::vector<std::string> &custom_names
   ):
    db(db),
    custom_names(custom_names)
   {
    for (const auto &[tid, tname]: db.get_tables())
     table_options[tid];
   }

   bool has_index() const
   {
    return indices.size() > 0;
   }

   bool has_unique_index() const
   {
    for (const auto &index: indices)
     if (index.unique)
      return true;
    return false;
   }

   bool has_single_row() const
   {
    for (const auto &options: table_options)
     if (options.second.single_row)
      return true;
    return false;
   }

   void set_name_space(std::vector<std::string> v)
   {
    name_space = std::move(v);
   }

   void set_single_row(Table_Id table_id, bool value)
   {
    table_options[table_id].single_row = value;
   }

   void add_index(Index index)
   {
    indices.emplace_back(std::move(index));
   }

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
 };
}

#endif
