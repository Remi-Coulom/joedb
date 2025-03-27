#include "joedb/ui/Command_Interpreter.h"

#include <chrono>

namespace joedb::concurrency
{
 class Client;
 class Pullonly_Client;
 class Writable_Interpreter;
}

namespace joedb::ui
{
 ////////////////////////////////////////////////////////////////////////////
 class Client_Command_Processor: public Command_Interpreter
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   concurrency::Pullonly_Client &client;
   const bool has_file;
   concurrency::Client * const push_client;

   void pull(std::ostream &out, std::chrono::milliseconds wait);
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
   Client_Command_Processor(concurrency::Pullonly_Client &client, bool has_file);
 };
}
