#include "joedb/io/dump.h"
#include "joedb/io/Journal_Pair.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static void process(Readonly_Journal &input, Journal_File &output)
 ////////////////////////////////////////////////////////////////////////////
 {
  pack(input, output);
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return process_journal_pair(argc, argv, joedb::process);
}