#ifndef joedb_Compiler_Options_declared
#define joedb_Compiler_Options_declared

#include "joedb/interpreted/Database.h"
#include "joedb/journal/Memory_File.h"

#include <string>
#include <vector>

namespace joedb
{
 /// @ingroup compiler
 class Compiler_Options
 {
  public:
   struct Index
   {
    bool unique;
    std::string name;
    Table_Id table_id;
    std::vector<Field_Id> field_ids;

    bool is_trigger(const Table_Id tid, const Field_Id fid) const
    {
     if (tid != table_id)
      return false;

     int index = int(field_ids.size());
     while (--index >= 0)
      if (field_ids[index] == fid)
       break;

     if (index < 0)
      return false;

     if (unique)
      for (auto id: field_ids)
       if (id > fid)
        return false;

     return true;
    }
   };

   struct Table_Options
   {
    bool single_row = false;
   };

  public:
   std::string exe_path;
   std::string output_path;
   std::string base_name;

   Database db;
   Memory_File schema_file;
   std::vector<std::string> custom_names;

   std::vector<std::string> name_space;
   std::vector<Index> indices;
   std::map<Table_Id, Table_Options> table_options;

  public:
   bool has_index() const
   {
    return indices.size() > 0;
   }

   bool has_blob() const
   {
    for (const auto &[table_id, table_name]: db.get_tables())
     for (const auto &[field_id, field_name]: db.get_fields(table_id))
      if (db.get_field_type(table_id, field_id).get_type_id() == Type::Type_Id::blob)
       return true;
    return false;
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
    for (const auto &[table_id, options]: table_options)
     if (options.single_row)
      return true;
    return false;
   }

   bool has_multi_row() const
   {
    for (const auto &[table_id, options]: table_options)
     if (!options.single_row)
      return true;
    return false;
   }

   bool is_unique_field_name(const std::string &field_name) const
   {
    int count = 0;

    for (const auto &[table_id, options]: table_options)
     for (const auto &[field_id, name]: db.get_fields(table_id))
      if (name == field_name)
       if (++count > 1)
        break;

    return count == 1;
   }

   bool has_table(const std::string &table_name) const
   {
    for (const auto &[table_id, name]: db.get_tables())
     if (name == table_name)
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
   const std::string &get_name_space_back() const
   {
    if (name_space.empty())
     throw Exception("missing name space");
    else
     return name_space.back();
   }
   const std::vector<Index> &get_indices() const {return indices;}
   const Table_Options &get_table_options(Table_Id table_id) const
   {
    return table_options.find(table_id)->second;
   }
 };
}

#endif
