#ifndef joedb_File_Buffer_declared
#define joedb_File_Buffer_declared

#include "joedb/error/assert.h"
#include "joedb/journal/File_Iterator.h"
#include "joedb/journal/Buffer.h"
#include "joedb/index_types.h"

#include <string>

namespace joedb
{
 /// @ingroup journal
 class File_Buffer: public File_Iterator
 {
  private:
   Buffer<12> buffer;

   size_t read_buffer_size;

   //////////////////////////////////////////////////////////////////////////
   bool buffer_has_write_data() const noexcept
   //////////////////////////////////////////////////////////////////////////
   {
    return buffer.index > 0 && !read_buffer_size;
   }

   //////////////////////////////////////////////////////////////////////////
   bool buffer_has_read_data() const
   //////////////////////////////////////////////////////////////////////////
   {
    return read_buffer_size;
   }

   //////////////////////////////////////////////////////////////////////////
   void read_buffer()
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_DEBUG_ASSERT(!buffer_has_write_data());
    JOEDB_DEBUG_ASSERT(buffer.index <= read_buffer_size);

    buffer.index = 0;
    read_buffer_size = File_Iterator::read(buffer.data, buffer.size);
    if (read_buffer_size == 0)
     file.reading_past_end_of_file();
   }

   //////////////////////////////////////////////////////////////////////////
   void write_buffer()
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_DEBUG_ASSERT(!buffer_has_read_data());

    File_Iterator::write(buffer.data, buffer.index);
    buffer.index = 0;
   }

   //////////////////////////////////////////////////////////////////////////
   void check_write_buffer()
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_DEBUG_ASSERT(!buffer_has_read_data());

    if (buffer.index >= buffer.size)
     write_buffer();
   }

  public:
   File_Buffer(Abstract_File &file);
   void flush();

   // set_position must be called when switching between write and read
   void set_position(int64_t position);

   int64_t get_position() const noexcept
   {
    return File_Iterator::get_position() - read_buffer_size + buffer.index;
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename T> void write(T x)
   //////////////////////////////////////////////////////////////////////////
   {
    static_assert(sizeof(T) <= decltype(buffer)::extra_size);
    buffer.write<T>(x);
    check_write_buffer();
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename T> T read()
   //////////////////////////////////////////////////////////////////////////
   {
    T result;
    read_data(reinterpret_cast<char *>(&result), sizeof(result));
    return result;
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename T> void compact_write(T x)
   //////////////////////////////////////////////////////////////////////////
   {
    buffer.compact_write<T>(x);
    check_write_buffer();
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename T> T compact_read()
   //////////////////////////////////////////////////////////////////////////
   {
    if (buffer.index + 8 <= read_buffer_size)
     return buffer.compact_read<T>();

    const uint8_t first_byte = read<uint8_t>();
    int extra_bytes = first_byte >> 5;
    T result = first_byte & 0x1f;
    while (--extra_bytes >= 0)
     result = T((result << 8) | read<uint8_t>());
    return result;
   }

   template<typename T> T read_strong_type()
   {
    return T(compact_read<typename underlying_type<T>::type>());
   }

   void write_reference(Record_Id id)
   {
    compact_write(to_underlying(id) + 1);
   }

   Record_Id read_reference()
   {
    return Record_Id(read_strong_type<Record_Id>() - 1);
   }

   void write_string(const std::string &s);
   std::string read_string();
   std::string safe_read_string(int64_t max_size);

   void write_blob(Blob blob)
   {
    compact_write<int64_t>(blob.get_position());
    compact_write<int64_t>(blob.get_size());
   }

   Blob read_blob()
   {
    const int64_t position = compact_read<int64_t>();
    const int64_t size = compact_read<int64_t>();
    return Blob(position, size);
   }

   //////////////////////////////////////////////////////////////////////////
   void write_data(const char *data, size_t n)
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_DEBUG_ASSERT(!buffer_has_read_data());

    if (n <= buffer.extra_size)
    {
     std::memcpy(buffer.data + buffer.index, data, n);
     buffer.index += n;
     check_write_buffer();
    }
    else
    {
     const size_t remaining = buffer.size + buffer.extra_size - buffer.index;

     if (n < remaining)
     {
      std::memcpy(buffer.data + buffer.index, data, n);
      buffer.index += n;
      check_write_buffer();
     }
     else
     {
      flush();
      File_Iterator::write(data, n);
     }
    }
   }

   //////////////////////////////////////////////////////////////////////////
   size_t read_data(char *data, const size_t n)
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_DEBUG_ASSERT(!buffer_has_write_data());
    JOEDB_DEBUG_ASSERT(buffer.index <= read_buffer_size);

    if (buffer.index + n <= read_buffer_size)
    {
     std::memcpy(data, buffer.data + buffer.index, n);
     buffer.index += n;
     return n;
    }
    else
    {
     size_t n0 = read_buffer_size - buffer.index;
     std::memcpy(data, buffer.data + buffer.index, n0);
     buffer.index += n0;

     if (n <= buffer.size)
     {
      read_buffer();

      while (n0 < n && buffer.index < read_buffer_size)
       data[n0++] = buffer.data[buffer.index++];
     }

     while (n0 < n)
     {
      const size_t actually_read = File_Iterator::read(data + n0, n - n0);
      if (actually_read == 0)
      {
       file.reading_past_end_of_file();
       break;
      }
      n0 += actually_read;
     }

     return n0;
    }
   }

   //////////////////////////////////////////////////////////////////////////
   void ignore(const int64_t n)
   //////////////////////////////////////////////////////////////////////////
   {
    if (buffer.index + n <= read_buffer_size)
     buffer.index += n;
    else
     set_position(get_position() + n);
   }

   ~File_Buffer();
 };
}

#endif
