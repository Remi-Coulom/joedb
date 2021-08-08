#include "joedb/concurrency/Shared_Local_File.h"
#include "joedb/concurrency/Mutex.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 Shared_Local_File::Shared_Local_File
 /////////////////////////////////////////////////////////////////////////////
 (
  Mutex &mutex,
  const std::string &file_name
 )
 {
  try
  {
   file.reset
   (
    new joedb::File
    (
     file_name,
     joedb::Open_Mode::write_existing_or_create_new
    )
   );
  }
  catch(const joedb::Exception &)
  {
   Posthumous_Catcher catcher;

   try
   {
    Mutex_Lock lock(mutex);
    lock.set_catcher(catcher);
    joedb::File readonly_file(file_name, joedb::Open_Mode::read_existing);
    file.reset(new joedb::Memory_File(joedb::Open_Mode::write_existing));
    file->copy(readonly_file);
    file->set_position(0);
   }
   catch(...)
   {
    file.reset(new joedb::Memory_File(joedb::Open_Mode::create_new));
   }

   catcher.rethrow();
  }
 }
}
