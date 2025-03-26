#ifndef joedb_Blob_Reader_Command_Processor_declared
#define joedb_Blob_Reader_Command_Processor_declared

#include "joedb/ui/Command_Processor.h"

namespace joedb
{
 class Blob_Reader;
}

namespace joedb::ui
{
 ////////////////////////////////////////////////////////////////////////////
 class Blob_Reader_Command_Processor: public Command_Processor
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Blob_Reader &blob_reader;

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) final;

  public:
   Blob_Reader_Command_Processor(Blob_Reader &blob_reader):
    blob_reader(blob_reader)
   {
   }
 };
}

#endif
