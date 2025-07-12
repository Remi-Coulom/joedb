#include "joedb/compiler/generator/RPC_Client_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 RPC_Client_h::RPC_Client_h
 (
  const Compiler_Options &options,
  const std::vector<Procedure> &procedures
 ):
  Generator(".", "rpc/Client.h", options),
  procedures(procedures)
 {
 }

 void RPC_Client_h::generate()
 {
  auto name_space = options.get_name_space();
  name_space.emplace_back("rpc");

  namespace_include_guard(out, "Client", name_space);
  out << "\n#include \"Signatures.h\"\n";
  out << "#include \"joedb/rpc/Client.h\"\n\n";

  namespace_open(out, name_space);

  out << R"RRR(
 /// Specialization of joedb::rpc::Client
 class Client: public joedb::rpc::Client
 {
  public:
   Client(joedb::Channel &channel):
    joedb::rpc::Client(channel, get_signatures())
   {
   }
)RRR";

  for (size_t i = 0; i < procedures.size(); i++)
  {
   const auto &procedure = procedures[i];
   out << "\n   void " << procedure.name << '(' << procedure.schema << "::Memory_Database &db)\n";
   out << "   {\n";
   out << "    db.soft_checkpoint();\n";
   out << "    call(" << i << ", db);\n";
   out << "   }\n";
  }

  out << " };\n";

  namespace_close(out, name_space);
  out << "\n#endif\n";
 }
}
