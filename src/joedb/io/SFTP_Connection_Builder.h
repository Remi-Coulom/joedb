#ifndef joedb_SFTP_Connection_Builder_declared
#define joedb_SFTP_Connection_Builder_declared

#include "joedb/io/Connection_Builder.h"
#include "joedb/concurrency/Readonly_File_Connection.h"
#include "joedb/journal/SFTP_File.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class SFTP_Connection_Data:
 /////////////////////////////////////////////////////////////////////////////
  private ssh::Session,
  private ssh::SFTP,
  private SFTP_File,
  public Readonly_File_Connection
 {
  public:
   SFTP_Connection_Data
   (
    const char *user,
    const char *host,
    const char *file_name,
    uint16_t ssh_port,
    int ssh_log_level
   ):
    ssh::Session(user, host, 22, 0),
    ssh::SFTP(*static_cast<ssh::Session *>(this)),
    SFTP_File(*static_cast<ssh::SFTP *>(this), file_name),
    Readonly_File_Connection(*static_cast<SFTP_File *>(this))
   {
   }
 };

 /////////////////////////////////////////////////////////////////////////////
 class SFTP_Connection_Builder: public Connection_Builder
 /////////////////////////////////////////////////////////////////////////////
 {
  public:
   const char *get_name() const final {return "sftp";}
   int get_min_parameters() const final {return 3;}
   int get_max_parameters() const final {return 5;}

   const char *get_parameters_description() const final
   {
    return "<user> <host> <file_name> [<ssh_port> [<ssh_log_level>]]";
   }

   std::unique_ptr<Connection> build(int argc, char **argv) final
   {
    const char * const user = argv[0];
    const char * const host = argv[1];
    const char * const file_name = argv[2];
    const unsigned ssh_port = argc > 3 ? std::atoi(argv[3]) : 22;
    const int ssh_log_level = argc > 4 ? std::atoi(argv[4]) : 0;

    return std::unique_ptr<Connection>
    (
     new SFTP_Connection_Data
     (
      user,
      host,
      file_name,
      ssh_port,
      ssh_log_level
     )
    );
   }
 };
}

#endif
