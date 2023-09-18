#ifndef joedb_Posthumous_Thrower_declared
#define joedb_Posthumous_Thrower_declared

namespace joedb
{
 class Posthumous_Catcher;

 ////////////////////////////////////////////////////////////////////////////
 class Posthumous_Thrower
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Posthumous_Catcher *catcher = nullptr;

  protected:
   void postpone_exception(const char *message = nullptr) noexcept;

  public:
   void set_catcher(Posthumous_Catcher &new_catcher) noexcept
   {
    catcher = &new_catcher;
   }
 };
}

#endif
