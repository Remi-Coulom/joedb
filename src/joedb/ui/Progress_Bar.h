#ifndef joedb_io_Progress_Bar_declared
#define joedb_io_Progress_Bar_declared

#include <cstdint>
#include <iosfwd>

namespace joedb
{
 /// @ingroup ui
 class Progress_Bar
 {
  private:
   static constexpr int length = 79;

   const int64_t total;
   std::ostream &out;

   int current_display;

  public:
   Progress_Bar(int64_t total, std::ostream &out);
   void print(int64_t current);
   void print_remaining(int64_t remaining) {print(total - remaining);}
   ~Progress_Bar();
 };
}

#endif
