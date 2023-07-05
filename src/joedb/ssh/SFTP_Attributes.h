#ifndef joedb_ssh_SFTP_Attributes_declared
#define joedb_ssh_SFTP_Attributes_declared

#include "joedb/ssh/ssh.h"

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  class SFTP_Attributes
  ///////////////////////////////////////////////////////////////////////////
  {
   private:
    const sftp_attributes attributes;

   public:
    SFTP_Attributes(sftp_attributes attributes): attributes(attributes)
    {
     check_not_null(attributes);
    }

    sftp_attributes get() const
    {
     return attributes;
    }

    ~SFTP_Attributes()
    {
     if (attributes)
      sftp_attributes_free(attributes);
    }
  };
 }
}

#endif
