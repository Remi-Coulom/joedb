#ifndef joedb_Value_declared
#define joedb_Value_declared

#include "Type.h"
#include "index_types.h"

#include <cassert>

namespace joedb
{
 class Value
 {
  private:
   Type::type_id_t type_id;
   union
   {
    int32_t int32;
    int64_t int64;
    record_id_t record_id;
   } u;
   std::string s;

  public:
   Value(): type_id(Type::type_id_t::null) {}
   Value(const std::string &s): type_id(Type::type_id_t::string), s(s) {}
   Value(int32_t i): type_id(Type::type_id_t::int32) {u.int32 = i;}
   Value(int64_t i): type_id(Type::type_id_t::int64) {u.int64 = i;}
   Value(record_id_t id): type_id(Type::type_id_t::reference)
   {
    u.record_id = id;
   }

   Value(Type::type_id_t type_id):
    type_id(type_id)
   {
    switch(type_id)
    {
     case Type::type_id_t::null: break;
     case Type::type_id_t::int32: u.int32 = 0; break;
     case Type::type_id_t::int64: u.int64 = 0; break;
     case Type::type_id_t::reference: u.record_id = 0; break;
     case Type::type_id_t::string: break;
    }
   }

   Type::type_id_t get_type_id() const {return type_id;}

   const std::string &get_string() const
   {
    assert(type_id == Type::type_id_t::string);
    return s;
   }

   int32_t get_int32() const
   {
    assert(type_id == Type::type_id_t::int32);
    return u.int32;
   }

   int64_t get_int64() const
   {
    assert(type_id == Type::type_id_t::int64);
    return u.int64;
   }

   record_id_t get_record_id() const
   {
    assert(type_id == Type::type_id_t::reference);
    return u.record_id;
   }
 };
}

#endif
