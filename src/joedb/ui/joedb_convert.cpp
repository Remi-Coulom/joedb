#include "joedb/ui/process_journal_pair.h"
#include "joedb/ui/main_exception_catcher.h"

namespace joedb::ui
{
 /////////////////////////////////////////////////////////////////////////////
 static void process
 /////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &input,
  Writable_Journal &output,
  int64_t checkpoint
 )
 {
  if (checkpoint <= 0)
   checkpoint = input.get_checkpoint_position();
  input.play_until(output, checkpoint);
  output.default_checkpoint();
 }

 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  return process_journal_pair(argc, argv, process);
 }
}

//////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
//////////////////////////////////////////////////////////////////////////////
{
 return joedb::ui::main_exception_catcher(joedb::ui::main, argc, argv);
}
