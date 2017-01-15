#ifndef joedb_Generic_File_declared
#define joedb_Generic_File_declared

#include <string>
#include <cstdint>

#include "joedb_assert.h"

namespace joedb
{
 enum class Open_Mode
 {
  read_existing,
  write_existing,
  create_new,
  write_existing_or_create_new
 };

 class Generic_File
 {
  private:
   static inline uint8_t is_big_endian()
   {
    const uint16_t n = 0x0100;
    return *(const uint8_t *)&n;
   }

  public:
   Generic_File()
   {
    write_buffer_index = 0;
    reset_read_buffer();
    position = 0;
   }

   Open_Mode get_mode() const {return mode;}

   bool is_end_of_file() const {return end_of_file;}

   // set_position must be called when switching between write and read
   void set_position(size_t position);
   size_t get_position() const {return position;}

   template<typename T>
   void write(T x)
   {
    W<T, sizeof(T)>::write(*this, x);
   }
   template<typename T>
   T read()
   {
    return R<T, sizeof(T)>::read(*this);
   }

   template<typename T>
   void compact_write(T x)
   {
    CW<T, sizeof(T)>::write(*this, x);
   }

   template<typename T>
   T compact_read()
   {
    uint8_t first_byte = getc();
    int extra_bytes = first_byte >> 5;
    T result = first_byte & 0x1f;
    while (extra_bytes--)
     result = T((result << 8) | getc());
    return result;
   }

   void write_string(const std::string &s);
   std::string read_string();
   std::string safe_read_string(size_t max_size);

   void flush(); // flushes the write buffer to the system
   void commit(); // flush and write to disk (fsync)

   virtual ~Generic_File() {}

   virtual int64_t get_size() const {return -1;} // -1 means no known size

  protected:
   Open_Mode mode;

   virtual size_t read_buffer() = 0;
   virtual void write_buffer() = 0;
   virtual int seek(size_t offset) = 0;
   virtual void sync() = 0;

   enum {buffer_size = (1 << 12)};
   enum {buffer_extra = 8};

   char buffer[buffer_size + buffer_extra];
   size_t write_buffer_index;

  private:
   size_t read_buffer_index;
   size_t read_buffer_size;
   bool end_of_file;
   size_t position;

   void putc(char c)
   {
    JOEDB_ASSERT(read_buffer_size == 0 && !end_of_file);
    buffer[write_buffer_index++] = c;
    position++;
   }

   uint8_t getc()
   {
    JOEDB_ASSERT(write_buffer_index == 0);

    if (read_buffer_index >= read_buffer_size)
    {
     read_buffer_size = read_buffer();
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
    write_buffer();
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
    static void write(Generic_File &file, T x)
    {
     file.putc(char(x));
     file.check_write_buffer();
    }
   };

   template<typename T>
   struct W<T, 2>
   {
    static void write(Generic_File &file, T x)
    {
     const char *p = reinterpret_cast<char *>(&x);
     if (is_big_endian())
     {
      file.putc(p[1]);
      file.putc(p[0]);
     }
     else
     {
      file.putc(p[0]);
      file.putc(p[1]);
     }
     file.check_write_buffer();
    }
   };

   template<typename T>
   struct W<T, 4>
   {
    static void write(Generic_File &file, T x)
    {
     const char *p = reinterpret_cast<char *>(&x);
     if (is_big_endian())
     {
      file.putc(p[3]);
      file.putc(p[2]);
      file.putc(p[1]);
      file.putc(p[0]);
     }
     else
     {
      file.putc(p[0]);
      file.putc(p[1]);
      file.putc(p[2]);
      file.putc(p[3]);
     }
     file.check_write_buffer();
    }
   };

   template<typename T>
   struct W<T, 8>
   {
    static void write(Generic_File &file, T x)
    {
     const char *p = reinterpret_cast<char *>(&x);
     if (is_big_endian())
     {
      file.putc(p[7]);
      file.putc(p[6]);
      file.putc(p[5]);
      file.putc(p[4]);
      file.putc(p[3]);
      file.putc(p[2]);
      file.putc(p[1]);
      file.putc(p[0]);
     }
     else
     {
      file.putc(p[0]);
      file.putc(p[1]);
      file.putc(p[2]);
      file.putc(p[3]);
      file.putc(p[4]);
      file.putc(p[5]);
      file.putc(p[6]);
      file.putc(p[7]);
     }
     file.check_write_buffer();
    }
   };

   template<typename T, int n> struct CW;

   template<typename T>
   struct CW<T, 2>
   {
    static void write(Generic_File &file, T x)
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
    static void write(Generic_File &file, T x)
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
    static void write(Generic_File &file, T x)
    {
     if (!(uint64_t(x) >> 32))
      file.compact_write<uint32_t>(uint32_t(x));
     else // TODO: should be optimized?
     {
      JOEDB_ASSERT(!(char(x >> 56) & 0xe0));
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
    static T read(Generic_File &file)
    {
     return T(file.getc());
    }
   };

   template<typename T>
   struct R<T, 2>
   {
    static T read(Generic_File &file)
    {
     T result;
     uint8_t *p = reinterpret_cast<uint8_t *>(&result);
     if (is_big_endian())
     {
      p[1] = file.getc();
      p[0] = file.getc();
     }
     else
     {
      p[0] = file.getc();
      p[1] = file.getc();
     }
     return result;
    }
   };

   template<typename T>
   struct R<T, 4>
   {
    static T read(Generic_File &file)
    {
     T result;
     uint8_t *p = reinterpret_cast<uint8_t *>(&result);
     if (is_big_endian())
     {
      p[3] = file.getc();
      p[2] = file.getc();
      p[1] = file.getc();
      p[0] = file.getc();
     }
     else
     {
      p[0] = file.getc();
      p[1] = file.getc();
      p[2] = file.getc();
      p[3] = file.getc();
     }
     return result;
    }
   };

   template<typename T>
   struct R<T, 8>
   {
    static T read(Generic_File &file)
    {
     T result;
     uint8_t *p = reinterpret_cast<uint8_t *>(&result);
     if (is_big_endian())
     {
      p[7] = file.getc();
      p[6] = file.getc();
      p[5] = file.getc();
      p[4] = file.getc();
      p[3] = file.getc();
      p[2] = file.getc();
      p[1] = file.getc();
      p[0] = file.getc();
     }
     else
     {
      p[0] = file.getc();
      p[1] = file.getc();
      p[2] = file.getc();
      p[3] = file.getc();
      p[4] = file.getc();
      p[5] = file.getc();
      p[6] = file.getc();
      p[7] = file.getc();
     }
     return result;
    }
   };
 };
}

#endif
