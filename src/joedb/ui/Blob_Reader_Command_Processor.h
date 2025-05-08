#ifndef joedb_Blob_Reader_Command_Processor_declared
#define joedb_Blob_Reader_Command_Processor_declared

#include "joedb/ui/Command_Processor.h"

namespace joedb
{
 class Buffered_File;

 /// @ingroup ui
 class Blob_Reader_Command_Processor: public Command_Processor
 {
  private:
   const Buffered_File &blob_reader;

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) override;

  public:
   Blob_Reader_Command_Processor(const Buffered_File &blob_reader):
    blob_reader(blob_reader)
   {
   }
 };
}

#endif
