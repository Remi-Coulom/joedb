#ifndef joedb_File_declared
#define joedb_File_declared

#include <cstdio>
#include <cstdint>

namespace joedb
{
 class File
 {
  public:
   enum class open_mode_t {read_existing, write_existing, create_new};

  private:
   FILE *file;
   open_mode_t open_mode;

  public:
   File(const char *file_name, open_mode_t open_mode);

   bool is_good() const {return file != 0;}
   open_mode_t get_open_mode() const {return open_mode;}

   void seek(int64_t offset);

   void write_uint8(uint8_t x);
   void write_uint16(uint16_t x);
   void write_uint32(uint32_t x);
   void write_uint64(uint64_t x);

   uint8_t read_uint8();
   uint16_t read_uint16();
   uint32_t read_uint32();
   uint64_t read_uint64();

   void commit();

   ~File();
 };
}

#endif
