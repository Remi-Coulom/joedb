#ifndef crazydb_Type_declared
#define crazydb_Type_declared

#include <string>

namespace crazydb
{
 class Type
 {
  private:
   enum Kind {string_id, int32_id, int64_id, reference_id};
   Kind kind;
   std::string table_name; // for reference only

   Type(Kind kind): kind(kind) {}
   Type(const char *table_name): kind(reference_id), table_name(table_name) {}

  public:
   bool is_reference() const {return kind == reference_id;}
   const std::string &get_table_name() const {return table_name;}

   static Type string() {return Type(string_id);}
   static Type int32() {return Type(int32_id);}
   static Type int64() {return Type(int64_id);}
   static Type reference(const char *table_name) {return Type(table_name);}
 };
}

#endif
