#ifndef joedb_io_Progress_Bar_declared
#define joedb_io_Progress_Bar_declared

#include <stdint.h>
#include <iosfwd>

namespace joedb
{
 /// @ingroup ui
 class Progress_Bar
 {
  private:
   static constexpr int length = 79;

   const int64_t total;
   std::ostream * const out;
   const int64_t threshold;

   int current_display;

  public:
   Progress_Bar(int64_t total, std::ostream *out, int64_t threshold = 16384);
   void print(int64_t current);
   void print_remaining(int64_t remaining) {print(total - remaining);}
   ~Progress_Bar();
 };
}

#endif
