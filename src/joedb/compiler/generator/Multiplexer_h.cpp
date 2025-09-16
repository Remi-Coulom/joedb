#include "joedb/compiler/generator/Multiplexer_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Multiplexer_h::Multiplexer_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Multiplexer.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Multiplexer_h::write(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  const Database &db = options.get_db();
  auto tables = db.get_tables();

  namespace_include_guard_open(out, "Multiplexer", options.get_name_space());

  out << R"RRR(
#include "Writable_Database.h"
#include "joedb/Multiplexer.h"
#include "joedb/Selective_Writable.h"

)RRR";


  namespace_open(out, options.get_name_space());

  out << R"RRR(
 namespace detail
 {
  class Multiplexer_Parent
  {
   protected:
    joedb::Selective_Writable selective_writable;

   public:
    Multiplexer_Parent(joedb::Writable_Journal &journal):
     selective_writable(journal, joedb::Selective_Writable::data_and_information)
    {
    }
  };
 }

 /// Write simultaneously to the database and the file (ignore schema changes)
 class Multiplexer:
  protected detail::Multiplexer_Parent,
  public joedb::Multiplexer
 {
  public:
   Multiplexer(Writable_Database &db):
    Multiplexer_Parent(db.journal),
    joedb::Multiplexer{db, selective_writable}
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  namespace_include_guard_close(out);
 }
}
