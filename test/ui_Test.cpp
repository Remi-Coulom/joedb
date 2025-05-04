#include "joedb/ui/Client_Command_Processor.h"
#include "joedb/ui/Client_Parser.h"

#include "gtest/gtest.h"

#include <fstream>

namespace joedb
{
 TEST(ui, client)
 {
  Client_Parser parser
  (
   Open_Mode::shared_write,
   Client_Parser::DB_Type::none
  );
  const std::array<const char *, 5> args
  {
   "--check", "none", "--db", "interpreted", "memory"
  };
  Client &client = parser.parse(args.size(), args.data());

  {
   std::ostringstream out;
   parser.print_help(out);
  }

  Writable_Client *writable_client = dynamic_cast<Writable_Client *>(&client);
  ASSERT_TRUE(writable_client);
  Writable_Client_Command_Processor processor(*writable_client);

  std::ostringstream out;
  std::ifstream in("client_test.joedbi");

  processor.set_prompt(true);
  processor.main_loop(in, out);

  std::ifstream reference_file("client_test.joedbi.out");
  ASSERT_TRUE(reference_file);
  std::ostringstream reference_string;
  reference_string << reference_file.rdbuf();

  EXPECT_EQ(reference_string.str(), out.str());
 }
}
