#include "joedb/io/process_journal_pair.h"
#include "joedb/io/main_exception_catcher.h"

namespace joedb
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
  try
  {
   if (checkpoint <= 0)
    checkpoint = input.get_checkpoint_position();
   input.play_until(output, checkpoint);
   output.checkpoint(Commit_Level::no_commit);
  }
  catch (const Exception &e)
  {
   output.checkpoint(Commit_Level::no_commit);
   throw;
  }
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
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
