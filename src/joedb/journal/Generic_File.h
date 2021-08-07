#ifndef joedb_Generic_File_declared
#define joedb_Generic_File_declared

#include <string>
#include <cstdint>
#include <vector>

#include "joedb/assert.h"
#include "joedb/is_big_endian.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 enum class Open_Mode
 ////////////////////////////////////////////////////////////////////////////
 {
  read_existing,
  write_existing,
  create_new,
  write_existing_or_create_new
 };

 ////////////////////////////////////////////////////////////////////////////
 class Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Async_Reader;
  friend class Async_Writer;

  // TODO: should create an intermediate "Buffered_File" class ?

  private:
   enum {buffer_size = (1 << 12)};
   enum {buffer_extra = 8};

   char buffer[buffer_size + buffer_extra];
   size_t write_buffer_index;
   size_t read_buffer_index;
   size_t read_buffer_size;
   bool end_of_file;
   int64_t position;
   int64_t slice_start;
   int64_t slice_length;

   void read_buffer()
   {
    read_buffer_size = raw_read(buffer, buffer_size);
    read_buffer_index = 0;
   }

   void write_buffer()
   {
    raw_write(buffer, write_buffer_index);
    write_buffer_index = 0;
   }

   void putc(char c)
   {
    JOEDB_ASSERT(read_buffer_size == 0 && !end_of_file);
    buffer[write_buffer_index++] = c;
    position++;
   }

   char getc()
   {
    JOEDB_ASSERT(write_buffer_index == 0);

    if (read_buffer_index >= read_buffer_size)
    {
     read_buffer();

     if (read_buffer_size == 0)
     {
      end_of_file = true;
      return 0;
     }
    }

    position++;
    return buffer[read_buffer_index++];
   }

   void reset_read_buffer()
   {
    read_buffer_index = 0;
    read_buffer_size = 0;
    end_of_file = false;
   }

   void check_write_buffer()
   {
    if (write_buffer_index >= buffer_size)
     write_buffer();
   }

   template<typename T, size_t n> struct W;

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

   template<typename T, size_t n> struct CW;

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

   Open_Mode mode;

  protected:
   void set_mode(Open_Mode mode)
   {
    this->mode = mode;
   }

   virtual size_t raw_read(char *buffer, size_t size) = 0;
   virtual void raw_write(const char *buffer, size_t size) = 0;
   virtual int raw_seek(int64_t offset) = 0;
   virtual int64_t raw_get_size() const = 0; // -1 means no known size
   virtual void sync() = 0;

   int seek(int64_t offset)
   {
    return raw_seek(offset + slice_start);
   }

  public:
   Generic_File(Open_Mode mode): mode(mode)
   {
    write_buffer_index = 0;
    reset_read_buffer();
    position = 0;
    slice_start = 0;
    slice_length = 0;
   }

   void set_slice(int64_t start, int64_t length)
   {
    slice_start = start;
    slice_length = length;
    set_position(0);
   }

   Open_Mode get_mode() const {return mode;}

   bool is_end_of_file() const {return end_of_file;}

   // set_position must be called when switching between write and read
   void set_position(int64_t position);
   int64_t get_position() const {return position;}
   void copy(Generic_File &source);

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
    uint8_t first_byte = uint8_t(getc());
    int extra_bytes = first_byte >> 5;
    T result = first_byte & 0x1f;
    while (extra_bytes--)
     result = T((result << 8) | uint8_t(getc()));
    return result;
   }

   void write_string(const std::string &s);
   std::string read_string();
   std::string safe_read_string(size_t max_size);

   void write_data(const char *data, size_t n)
   {
    flush();
    raw_write(data, n);
    position += n;
   }

   void read_data(char *data, size_t n)
   {
    JOEDB_ASSERT(write_buffer_index == 0);

    if (read_buffer_index + n <= read_buffer_size)
    {
     for (size_t i = 0; i < n; i++)
      data[i] = buffer[read_buffer_index++];
     position += n;
    }
    else
    {
     size_t n0 = 0;

     while (n0 < n && read_buffer_index < read_buffer_size)
     {
      data[n0++] = buffer[read_buffer_index++];
      position++;
     }

     if (n <= buffer_size)
     {
      read_buffer();

      while (n0 < n && read_buffer_index < read_buffer_size)
      {
       data[n0++] = buffer[read_buffer_index++];
       position++;
      }

      if (n0 < n)
       end_of_file = true;
     }
     else
     {
      while (true)
      {
       const size_t actually_read = raw_read(data + n0, n - n0);

       position += actually_read;
       n0 += actually_read;

       if (n0 == n || actually_read == 0)
        break;
      }

      if (n0 < n)
       end_of_file = true;
     }
    }
   }

   void flush(); // flushes the write buffer to the system
   void commit(); // flush and write to disk (fsync)

   virtual ~Generic_File() noexcept(false) {}

   int64_t get_size() const
   {
    if (slice_length)
     return slice_length;
    else
     return raw_get_size();
   }

   template<typename T, size_t n> struct R;

   //////////////////////////////////////////////////////////////////////////
   template<typename T> struct R<T, 1>
   //////////////////////////////////////////////////////////////////////////
   {
    static void swap(T &x)
    {
    }

    static T read(Generic_File &file)
    {
     return T(file.getc());
    }
   };

   //////////////////////////////////////////////////////////////////////////
   template<typename T> struct R<T, 2>
   //////////////////////////////////////////////////////////////////////////
   {
    static void swap(T &x)
    {
     char *p = reinterpret_cast<char *>(&x);
     std::swap(p[0], p[1]);
    }

    static T read(Generic_File &file)
    {
     T result;
     file.read_data(reinterpret_cast<char *>(&result), 2);
     if (is_big_endian())
      swap(result);
     return result;
    }
   };

   //////////////////////////////////////////////////////////////////////////
   template<typename T> struct R<T, 4>
   //////////////////////////////////////////////////////////////////////////
   {
    static void swap(T &x)
    {
     char *p = reinterpret_cast<char *>(&x);
     std::swap(p[0], p[3]);
     std::swap(p[1], p[2]);
    }

    static T read(Generic_File &file)
    {
     T result;
     file.read_data(reinterpret_cast<char *>(&result), 4);
     if (is_big_endian())
      swap(result);
     return result;
    }
   };

   //////////////////////////////////////////////////////////////////////////
   template<typename T> struct R<T, 8>
   //////////////////////////////////////////////////////////////////////////
   {
    static void swap(T &x)
    {
     char *p = reinterpret_cast<char *>(&x);
     std::swap(p[0], p[7]);
     std::swap(p[1], p[6]);
     std::swap(p[2], p[5]);
     std::swap(p[3], p[4]);
    }

    static T read(Generic_File &file)
    {
     T result;
     file.read_data(reinterpret_cast<char *>(&result), 8);
     if (is_big_endian())
      swap(result);
     return result;
    }
   };
 };
}

#endif
