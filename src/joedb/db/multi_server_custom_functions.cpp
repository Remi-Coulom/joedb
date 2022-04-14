#include "joedb/db/multi_server.h"

namespace joedb
{
 namespace multi_server
 {
  ////////////////////////////////////////////////////////////////////////////
  void Generic_File_Database::set_default_ssh_backup
  ////////////////////////////////////////////////////////////////////////////
  (
   Generic_File_Database &db
  )
  {
   for (auto server: db.get_server_table())
    db.set_ssh_backup(server, db.null_ssh_connection());
  }
 }
}
