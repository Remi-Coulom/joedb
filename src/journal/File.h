#ifndef joedb_File_declared
#define joedb_File_declared

#include <cstdio>
#include <string>
#include <cassert>
#include <cstdint>

namespace joedb
{
 class File
 {
  public:
   enum class mode_t {read_existing, write_existing, create_new};

   File(): file(0) {}
   File(const char *file_name, mode_t mode) {open(file_name, mode);}
   bool open(const char *file_name, mode_t mode);

   bool is_good() const {return file != 0;}
   mode_t get_mode() const {return mode;}
   bool is_end_of_file() const {return end_of_file;}

   // set_position must be called when switching between write and read
   void set_position(uint64_t position);
   uint64_t get_position() const {return position;}

   template<typename T> void write(T x)
   {
    W<T, sizeof(T)>::write(*this, x);
   }
   template<typename T> T read()
   {
    return R<T, sizeof(T)>::read(*this);
   }
   template<typename T> void compact_write(T x)
   {
    CW<T, sizeof(T)>::write(*this, x);
   }
   template<typename T> T compact_read()
   {
    return CR<T, sizeof(T)>::read(*this);
   }

   void write_string(const std::string &s);
   std::string read_string();

   void flush(); // flushes the write buffer
   void commit(); // write to disk (fsync)

   ~File();

  private:
   FILE *file;
   mode_t mode;

   enum {buffer_size = (1 << 16)};
   enum {buffer_extra = 8};

   char buffer[buffer_size + buffer_extra];
   size_t write_buffer_index;
   size_t read_buffer_index;
   size_t read_buffer_size;
   bool end_of_file;
   uint64_t position;

   void putc(char c)
   {
    assert(read_buffer_size == 0 && !end_of_file);
    buffer[write_buffer_index++] = c;
    position++;
   }

   uint8_t getc()
   {
    assert(write_buffer_index == 0);

    if (read_buffer_index >= read_buffer_size)
    {
     read_buffer_size = std::fread(buffer, 1, buffer_size, file);
     read_buffer_index = 0;
    }

    if (read_buffer_index < read_buffer_size)
    {
     position++;
     return (uint8_t)buffer[read_buffer_index++];
    }
    else
    {
     end_of_file = true;
     return 0;
    }
   }

   void reset_read_buffer()
   {
    read_buffer_index = 0;
    read_buffer_size = 0;
    end_of_file = false;
   }

   void flush_write_buffer()
   {
    std::fwrite(buffer, 1, write_buffer_index, file);
    write_buffer_index = 0;
   }

   void check_write_buffer()
   {
    if (write_buffer_index >= buffer_size)
     flush_write_buffer();
   }

   template<typename T, int n> struct W;
   template<typename T, int n> struct R;

   template<typename T>
   struct W<T, 1>
   {
    static void write(File &file, T x)
    {
     file.putc(char(x));
     file.check_write_buffer();
    }
   };

   template<typename T>
   struct W<T, 2>
   {
    static void write(File &file, T x)
    {
     file.putc(char(x >>  0));
     file.putc(char(x >>  8));
     file.check_write_buffer();
    }
   };

   template<typename T>
   struct W<T, 4>
   {
    static void write(File &file, T x)
    {
     file.putc(char(x >>  0));
     file.putc(char(x >>  8));
     file.putc(char(x >> 16));
     file.putc(char(x >> 24));
     file.check_write_buffer();
    }
   };

   template<typename T>
   struct W<T, 8>
   {
    static void write(File &file, T x)
    {
     file.putc(char(x >>  0));
     file.putc(char(x >>  8));
     file.putc(char(x >> 16));
     file.putc(char(x >> 24));
     file.putc(char(x >> 32));
     file.putc(char(x >> 40));
     file.putc(char(x >> 48));
     file.putc(char(x >> 56));
     file.check_write_buffer();
    }
   };

   template<typename T, int n> struct CW;

   template<typename T>
   struct CW<T, 2>
   {
    static void write(File &file, T x)
    {
     uint8_t b1 = uint8_t(x >> 8);
     uint8_t b0 = uint8_t(x);

     if (b1)
     {
      if (b1 < 32)
       file.putc(char(32 | b1));
      else
      {
       file.putc(64);
       file.putc(char(b1));
      }
     }
     else
      if (b0 >= 32)
       file.putc(32);

     file.putc(char(b0));
     file.check_write_buffer();
    }
   };

   template<typename T>
   struct CW<T, 4>
   {
    static void write(File &file, T x)
    {
     if (!(uint32_t(x) >> 16))
      file.compact_write<uint16_t>(uint16_t(x));
     else
     {
      uint8_t b1 = uint8_t(x >> 24);
      uint8_t b0 = uint8_t(x >> 16);

      if (b1)
      {
       if (b1 < 32)
        file.putc(char(96 | b1));
       else
       {
        file.putc(char(128));
        file.putc(char(b1));
       }
       file.putc(char(b0));
      }
      else
       if (b0 < 32)
        file.putc(char(64 | b0));
       else
       {
        file.putc(char(96));
        file.putc(char(b0));
       }

      file.putc(char(x >> 8));
      file.putc(char(x));
      file.check_write_buffer();
     }
    }
   };

   template<typename T>
   struct CW<T, 8>
   {
    static void write(File &file, T x)
    {
     if (!(uint64_t(x) >> 32))
      file.compact_write<uint32_t>(uint32_t(x));
     else
     {
      assert(!(char(x >> 56) & 0xe0));
      file.putc(char(0xe0) | char(x >> 56));
      file.putc(char(x >> 48));
      file.putc(char(x >> 40));
      file.putc(char(x >> 32));
      file.putc(char(x >> 24));
      file.putc(char(x >> 16));
      file.putc(char(x >> 8));
      file.putc(char(x));
      file.check_write_buffer();
     }
    }
   };

   template<typename T>
   struct R<T, 1>
   {
    static T read(File &file)
    {
     return T(file.getc());
    }
   };

   template<typename T>
   struct R<T, 2>
   {
    static T read(File &file)
    {
     return T((uint16_t(file.getc()) <<  0) |
              (uint16_t(file.getc()) <<  8));
    }
   };

   template<typename T>
   struct R<T, 4>
   {
    static T read(File &file)
    {
     return T((uint32_t(file.getc()) <<  0) |
              (uint32_t(file.getc()) <<  8) |
              (uint32_t(file.getc()) << 16) |
              (uint32_t(file.getc()) << 24));
    }
   };

   template<typename T>
   struct R<T, 8>
   {
    static T read(File &file)
    {
     return T((uint64_t(file.getc()) <<  0) |
              (uint64_t(file.getc()) <<  8) |
              (uint64_t(file.getc()) << 16) |
              (uint64_t(file.getc()) << 24) |
              (uint64_t(file.getc()) << 32) |
              (uint64_t(file.getc()) << 40) |
              (uint64_t(file.getc()) << 48) |
              (uint64_t(file.getc()) << 56));
    }
   };

   template<typename T, int n>
   struct CR
   {
    static T read(File &file)
    {
     uint8_t first_byte = file.getc();
     int extra_bytes = first_byte >> 5;
     T result = first_byte & 0x1f;
     while (extra_bytes--)
      result = T((result << 8) | file.getc());
     return result;
    }
   };
 };
}

#endif
