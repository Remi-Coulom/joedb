#include "joedb/io/Command_Processor.h"
#include "joedb/io/Command_Interpreter.h"

namespace joedb
{
 class Client;
 class Interpreted_Client_Data;
 class Journal_Client_Data;
 class Writable_Interpreter;

 ////////////////////////////////////////////////////////////////////////////
 class Client_Command_Processor: public Command_Processor
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Client &client;

   static void run_transaction(Writable_Interpreter &interpreter);

   void pull(std::ostream &out);

   Status process_command
   (
    const std::string &command,
    std::istream &iss,
    std::ostream &out
   ) override;

  public:
   Client_Command_Processor(Client &client);
 };
}
