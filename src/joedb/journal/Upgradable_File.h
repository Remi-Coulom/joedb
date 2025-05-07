#ifndef joedb_Upgradable_File_declared
#define joedb_Upgradable_File_declared

#include <stddef.h>
#include <stdint.h>

namespace joedb
{
 /// @ingroup journal
 template <typename Parent> class Upgradable_File: public Parent
 {
  private:
   void pwrite(const char *data, size_t size, int64_t offset) override {}
   int64_t get_size() const override {return -1;}

  public:
   template<class... Arguments> Upgradable_File(Arguments &&... arguments):
    Parent(arguments...)
   {
    this->make_writable();
   }
 };
}

#endif
