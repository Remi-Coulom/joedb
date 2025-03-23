#ifndef joedb_Upgradable_File_declared
#define joedb_Upgradable_File_declared

#include <stddef.h>
#include <stdint.h>

namespace joedb
{
 template <typename Parent>
 ////////////////////////////////////////////////////////////////////////////
 class Upgradable_File: public Parent
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   void pwrite(const char *data, size_t size, int64_t offset) override {}

  public:
   template<class... Arguments> Upgradable_File(Arguments &&... arguments):
    Parent(arguments...)
   {
    this->make_writable();
   }
 };
}

#endif
