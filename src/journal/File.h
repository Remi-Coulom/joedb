#ifndef joedb_File_declared
#define joedb_File_declared

#include <cstdio>
#include <cstdint>

namespace joedb
{
 class File
 {
  private:
   FILE *file;
   const bool read_only;
   bool created_new;
   int64_t file_size;

  public:
   File(const char *file_name, bool read_only);

   bool is_good() const;
   bool is_created_new() const;
   bool is_read_only() const;
   int64_t get_file_size() const;

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
