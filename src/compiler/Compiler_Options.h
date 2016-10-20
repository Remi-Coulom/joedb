#ifndef joedb_Compiler_Options_declared
#define joedb_Compiler_Options_declared

#include <string>
#include <vector>

#include "index_types.h"

namespace joedb
{
 class Database;

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

  private:
   const Database &db;

   std::string namespace_name;
   std::vector<Index> indices;

  public:
   Compiler_Options(const Database &db): db(db) {}

   void set_namespace_name(const std::string &s) {namespace_name = s;}
   void add_index(const Index &index) {indices.push_back(index);}

   const Database &get_db() const {return db;}
   const std::string &get_namespace_name() const {return namespace_name;}
   const std::vector<Index> &get_indices() const {return indices;}
 };
}

#endif
