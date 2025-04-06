#include "joedb/ui/Command_Interpreter.h"

#include <chrono>

namespace joedb
{
 class Client;

 /// @ingroup ui
 class Client_Command_Processor: public Command_Interpreter
 {
  private:
   Client &client;

   void pull(std::ostream &out, std::chrono::milliseconds wait);
   void print_status(std::ostream &out);
   static void sleep(int seconds, std::ostream &out);

   void write_prompt(std::ostream &out) const override;

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) override;

  public:
   Client_Command_Processor(Client &client): client(client) {}
 };
}
