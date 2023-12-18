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

   static void run_transaction
   (
    Writable_Interpreter &interpreter,
    std::istream &in,
    std::ostream &out
   );

   void pull(std::ostream &out);
   void print_status(std::ostream &out);
   static void sleep(int seconds, std::ostream &out);

   bool is_readonly_data() const;
   bool has_db() const;

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
