#ifndef crazydb_Type_declared
#define crazydb_Type_declared

#include <string>

namespace crazydb
{
 class Type
 {
  public:
   enum Kind {null_id, string_id, int32_id, int64_id, reference_id};

  private:
   Kind kind;
   std::string table_name; // for reference only

   Type(Kind kind): kind(kind) {}
   Type(const char *table_name): kind(reference_id), table_name(table_name) {}

  public:
   Kind get_kind() const {return kind;}
   const std::string &get_table_name() const {return table_name;}

   Type(): kind(null_id) {}
   static Type string() {return Type(string_id);}
   static Type int32() {return Type(int32_id);}
   static Type int64() {return Type(int64_id);}
   static Type reference(const char *table_name) {return Type(table_name);}
 };
}

#endif
