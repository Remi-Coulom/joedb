#ifndef joedb_ssh_SFTP_Attributes_declared
#define joedb_ssh_SFTP_Attributes_declared

#include "joedb/error/assert.h"

#include <libssh/sftp.h>

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
     JOEDB_RELEASE_ASSERT(attributes);
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
