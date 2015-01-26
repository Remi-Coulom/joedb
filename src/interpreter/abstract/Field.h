#ifndef joedb_Field_declared
#define joedb_Field_declared

#include <string>
#include <deque>
#include <cassert>

#include "Type.h"
#include "index_types.h"

namespace joedb
{
 class Field
 {
  private:
   const std::string name;
   const Type type;

   // TODO: bad, they are copied around
   // should be stored separately inside table?
   std::deque<std::string> string_column;
   std::deque<uint32_t> int32_column;
   std::deque<uint64_t> int64_column;
   std::deque<record_id_t> reference_column;

  public:
   Field(const std::string &name, const Type &type, size_t size):
    name(name),
    type(type)
   {
    resize(size);
   }

   const std::string &get_name() const {return name;}
   const Type &get_type() const {return type;}

   void resize(size_t size)
   {
    switch (type.get_type_id())
    {
     case Type::type_id_t::null:
     break;

     case Type::type_id_t::string:
      string_column.resize(size);
     break;

     case Type::type_id_t::int32:
      int32_column.resize(size);
     break;

     case Type::type_id_t::int64:
      int64_column.resize(size);
     break;

     case Type::type_id_t::reference:
      reference_column.resize(size);
     break;
    }
   }

   const std::string &get_string(record_id_t record_id) const
   {
    assert(type.get_type_id() == Type::type_id_t::string);
    return string_column[record_id - 1];
   }

   void set_string(record_id_t record_id, const std::string &s)
   {
    assert(type.get_type_id() == Type::type_id_t::string);
    string_column[record_id - 1] = s;
   }
 };
}

#endif
