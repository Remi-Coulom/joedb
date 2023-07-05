#include "SFTP.h"

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  char const * const SFTP::error_message[] =
  ///////////////////////////////////////////////////////////////////////////
  {
   "OK",
   "end of file",
   "no such file",
   "permission denied",
   "failure",
   "bad message",
   "no connection",
   "connection lost",
   "op unsupported",
   "invalid handle",
   "no such path",
   "file already exists",
   "write protect",
   "no media"
  };

  ///////////////////////////////////////////////////////////////////////////
  void SFTP::throw_error(const char *message) const
  ///////////////////////////////////////////////////////////////////////////
  {
   const int error = sftp_get_error(sftp);

   char const * const error_string =
    error < int(sizeof(error_message) / sizeof(*error_message)) ?
    error_message[error] :
    "unknown error";

   throw joedb::Exception
   (
    std::string(message) + std::string(error_string)
   );
  }
 }
}
