#ifndef joedb_Value_declared
#define joedb_Value_declared

#include "Type.h"
#include "index_types.h"

namespace joedb
{
 class Value
 {
  private:
   bool initialized;
   union
   {
    int32_t int32;
    int64_t int64;
    record_id_t record_id;
   } u;
   std::string s;

   // TODO: debug option to check get corresponds to constructor

  public:
   Value(): initialized(false) {}
   Value(int32_t i): initialized(true) {u.int32 = i;}
   Value(int64_t i): initialized(true) {u.int64 = i;}
   Value(record_id_t id): initialized(true) {u.record_id = id;}
   Value(const std::string &s): initialized(true), s(s) {}

   bool is_initialized() const {return initialized;}
   int32_t get_int32() const {return u.int32;}
   int64_t get_int64() const {return u.int64;}
   record_id_t get_record_id() const {return u.record_id;}
   const std::string &get_string() const {return s;}
 };
}

#endif
