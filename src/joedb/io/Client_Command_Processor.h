#include "joedb/io/Command_Interpreter.h"

namespace joedb
{
 class Client;
 class Writable_Interpreter;

 ////////////////////////////////////////////////////////////////////////////
 class Client_Command_Processor: public Command_Interpreter
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Client &client;

   void run_transaction
   (
    Writable_Interpreter &interpreter,
    std::istream &in,
    std::ostream &out
   );

   void pull(std::ostream &out);
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
   Client_Command_Processor(Client &client);
 };
}
