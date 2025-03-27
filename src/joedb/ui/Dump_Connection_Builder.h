#ifndef joedb_Dump_Connection_Builder
#define joedb_Dump_Connection_Builder

#include "joedb/concurrency/Writable_Connection.h"
#include "joedb/ui/Connection_Builder.h"
#include "joedb/Multiplexer.h"
#include "joedb/interpreted/Database_Schema.h"

namespace joedb::ui
{
 ////////////////////////////////////////////////////////////////////////////
 template <typename Dump_Writable> class Dump_Connection_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   interpreted::Database_Schema schema;
   Dump_Writable dump_writable;
   Multiplexer multiplexer;

  public:
   Dump_Connection_Data(std::ostream &out):
    dump_writable(out, schema),
    multiplexer{dump_writable, schema}
   {
   }
 };

 template<typename Dump_Writable>
 ////////////////////////////////////////////////////////////////////////////
 class Dump_Connection_Builder:
 ////////////////////////////////////////////////////////////////////////////
  private Dump_Connection_Data<Dump_Writable>,
  private concurrency::Writable_Connection,
  public Connection_Builder
 {
  private:
   bool mute_during_handshake;

  public:
   Dump_Connection_Builder(std::ostream &out):
    Dump_Connection_Data<Dump_Writable>(out),
    Writable_Connection(this->multiplexer),
    mute_during_handshake(false)
   {
   }

   int get_max_parameters() const final {return 1;}
   const char *get_name() const final {return this->dump_writable.get_name();}
   const char *get_parameters_description() const final {return "[tail]";}

   //////////////////////////////////////////////////////////////////////////
   Pullonly_Connection &build(int argc, char **argv, Buffered_File *file) final
   //////////////////////////////////////////////////////////////////////////
   {
    mute_during_handshake = argc > 0;

    if (mute_during_handshake)
     this->multiplexer.set_start_index(1);

    return *this;
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t handshake
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &client_journal,
    bool contentcheck
   ) override
   {
    const int64_t result = Writable_Connection::handshake
    (
     client_journal,
     contentcheck
    );

    if (mute_during_handshake)
     this->multiplexer.set_start_index(0);

    return result;
   }
 };
}

#endif
