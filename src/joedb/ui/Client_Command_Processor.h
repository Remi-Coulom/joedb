#include "joedb/ui/Command_Interpreter.h"
#include "joedb/concurrency/Readonly_Client.h"
#include "joedb/concurrency/Writable_Client.h"

#include <chrono>

namespace joedb
{
 class Client;

 /// @ingroup ui
 class Client_Command_Processor: public Command_Interpreter
 {
  private:
   void pull(std::ostream &out, std::chrono::milliseconds wait);
   void print_status(std::ostream &out);

  protected:
   Client &client;

   static void sleep(int seconds, std::ostream &out);
   virtual std::string get_name() const {return "client";}

  public:
   Client_Command_Processor(Client &client): client(client) {}

   void write_prompt(std::ostream &out) const override;

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) override;
 };

 /// @ingroup ui
 class Readonly_Client_Command_Processor: public Client_Command_Processor
 {
  private:
   Readonly_Client &get_readonly_client()
   {
    return static_cast<Readonly_Client&>(Client_Command_Processor::client);
   }

   std::string get_name() const override {return "readonly_client";}

  public:
   Readonly_Client_Command_Processor(Readonly_Client &client):
    Client_Command_Processor(client)
   {
   }

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) override;
 };

 /// @ingroup ui
 class Writable_Client_Command_Processor: public Client_Command_Processor
 {
  private:
   Writable_Client &get_writable_client()
   {
    return static_cast<Writable_Client&>(Client_Command_Processor::client);
   }

   std::string get_name() const override {return "writable_client";}

  public:
   Writable_Client_Command_Processor(Writable_Client &client):
    Client_Command_Processor(client)
   {
   }

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) override;
 };
}
