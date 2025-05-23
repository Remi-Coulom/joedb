#include "joedb/ssh/SFTP.h"

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  char const * const SFTP_Allocation::error_message[] =
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
  void SFTP_Allocation::throw_error(const char *message) const
  ///////////////////////////////////////////////////////////////////////////
  {
   const size_t error = size_t(sftp_get_error(sftp));

   char const * const error_string = error < std::size(error_message)
    ? error_message[error]
    : "unknown error";

   throw Exception
   (
    std::string(message) + std::string(error_string)
   );
  }
 }
}
