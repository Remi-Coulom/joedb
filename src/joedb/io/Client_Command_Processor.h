#include "joedb/io/Command_Processor.h"
#include "joedb/io/Command_Interpreter.h"

namespace joedb
{
 class Client;
 class Interpreted_Client_Data;
 class Journal_Client_Data;
 class Writable_Command_Processor;

 ////////////////////////////////////////////////////////////////////////////
 class Client_Command_Processor: public Command_Processor
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Client &client;
   Interpreted_Client_Data * const interpreted_client_data;
   Journal_Client_Data * const journal_client_data;

   static void run_transaction
   (
    Command_Interpreter &interpreter,
    Writable_Command_Processor &command_processor
   );

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
