#include "joedb/io/Command_Interpreter.h"

namespace joedb
{
 class Client;
 class Pullonly_Client;
 class Writable_Interpreter;

 ////////////////////////////////////////////////////////////////////////////
 class Client_Command_Processor: public Command_Interpreter
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Pullonly_Client &client;
   Client * const push_client;

   void pull(std::ostream &out, int64_t wait_milliseconds);
   void print_status(std::ostream &out);
   static void sleep(int seconds, std::ostream &out);

   bool is_readonly_data() const;

   void write_prompt(std::ostream &out) const override;

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) override;

  public:
   Client_Command_Processor(Pullonly_Client &client);
 };
}
