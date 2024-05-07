#ifndef joedb_Generic_File_declared
#define joedb_Generic_File_declared

#include "joedb/assert.h"
#include "joedb/Blob.h"
#include "joedb/Posthumous_Thrower.h"
#include "joedb/journal/Open_Mode.h"
#include "joedb/journal/SHA_256.h"
#include "joedb/journal/Abstract_File.h"
#include "joedb/journal/Buffer.h"

#include <string>
#include <vector>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Generic_File:
 ////////////////////////////////////////////////////////////////////////////
  public Abstract_File,
  public Posthumous_Thrower,
  public Blob_Reader,
  public Blob_Writer
 {
  friend class Async_Reader;
  friend class Async_Writer;

  private:
   Buffer<12> buffer;

   size_t read_buffer_size;
   bool end_of_file;

   //////////////////////////////////////////////////////////////////////////
   bool buffer_has_write_data() const
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
    JOEDB_ASSERT(!buffer_has_write_data());
    JOEDB_ASSERT(buffer.index <= read_buffer_size);

    read_buffer_size = pos_read(buffer.data, buffer.size);
    if (read_buffer_size == 0)
     end_of_file = true;

    buffer.index = 0;
   }

   //////////////////////////////////////////////////////////////////////////
   void write_buffer()
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_ASSERT(!buffer_has_read_data());

    pos_write(buffer.data, buffer.index);
    buffer.index = 0;
   }

   //////////////////////////////////////////////////////////////////////////
   void check_write_buffer()
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_ASSERT(!buffer_has_read_data());

    if (buffer.index >= buffer.size)
     write_buffer();
   }

   Open_Mode mode;
   const bool shared;
   bool locked_tail;

  protected:
   void destructor_flush() noexcept;

  public:
   Generic_File(Open_Mode mode);
   void flush();

   //////////////////////////////////////////////////////////////////////////
   void commit()
   //////////////////////////////////////////////////////////////////////////
   {
    flush();
    raw_sync();
   }

   //////////////////////////////////////////////////////////////////////////
   void set_mode(Open_Mode new_mode)
   //////////////////////////////////////////////////////////////////////////
   {
    mode = new_mode;
   }

   static constexpr int64_t last_position = (1ULL << 63) - 1;

   void exclusive_lock_tail()
   {
    exclusive_lock(last_position, 1);
    locked_tail = true;
   }

   void unlock_tail()
   {
    unlock(last_position, 1);
    locked_tail = false;
   }

   bool tail_is_locked() const
   {
    return locked_tail;
   }

   void shared_lock_head() {shared_lock(0, 1);}
   void exclusive_lock_head() {exclusive_lock(0, 1);}
   void unlock_head() {unlock(0, 1);}

   Open_Mode get_mode() const {return mode;}
   bool is_shared() const {return shared;}

   bool is_end_of_file() const {return end_of_file;}

   // set_position must be called when switching between write and read
   void set_position(int64_t position);

   int64_t get_position() const noexcept
   {
    return get_file_position() - read_buffer_size + buffer.index;
   }

   //////////////////////////////////////////////////////////////////////////
   virtual void copy_to
   //////////////////////////////////////////////////////////////////////////
   (
    Generic_File &destination,
    int64_t start,
    int64_t size
   );

   //////////////////////////////////////////////////////////////////////////
   template<typename T> void write(T x)
   //////////////////////////////////////////////////////////////////////////
   {
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
    if (read_buffer_size - buffer.index >= sizeof(T))
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
    return T(compact_read<typename std::underlying_type<T>::type>());
   }

   void write_reference(Record_Id id)
   {
    compact_write(to_underlying(id));
   }

   Record_Id read_reference()
   {
    return Record_Id(compact_read<std::underlying_type<Record_Id>::type>());
   }

   void write_string(const std::string &s);
   std::string read_string();
   void write_blob(Blob blob) {compact_write<int64_t>(blob.get_position());}
   Blob read_blob() {return Blob(compact_read<int64_t>());}
   std::string safe_read_string(int64_t max_size);

   //////////////////////////////////////////////////////////////////////////
   void write_data(const char *data, size_t n)
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_ASSERT(!end_of_file && !buffer_has_read_data());

    if (n <= buffer.extra_size)
    {
     std::copy_n(data, n, buffer.data + buffer.index);
     buffer.index += n;
     check_write_buffer();
    }
    else
    {
     const size_t remaining = buffer.size + buffer.extra_size - buffer.index;

     if (n < remaining)
     {
      std::copy_n(data, n, buffer.data + buffer.index);
      buffer.index += n;
      check_write_buffer();
     }
     else
     {
      flush();
      pos_write(data, n);
     }
    }
   }

   //////////////////////////////////////////////////////////////////////////
   size_t read_data(char *data, const size_t n)
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_ASSERT(!buffer_has_write_data());
    JOEDB_ASSERT(buffer.index <= read_buffer_size);

    if (buffer.index + n <= read_buffer_size)
    {
     std::copy_n(buffer.data + buffer.index, n, data);
     buffer.index += n;
     return n;
    }
    else
    {
     size_t n0 = read_buffer_size - buffer.index;
     std::copy_n(buffer.data + buffer.index, n0, data);
     buffer.index += n0;

     if (n <= buffer.size)
     {
      read_buffer();

      while (n0 < n && buffer.index < read_buffer_size)
       data[n0++] = buffer.data[buffer.index++];
     }

     while (n0 < n)
     {
      const size_t actually_read = pos_read(data + n0, n - n0);
      if (actually_read == 0)
      {
       end_of_file = true;
       break;
      }
      n0 += actually_read;
     }

     return n0;
    }
   }

   SHA_256::Hash get_hash(int64_t start, int64_t size);
   SHA_256::Hash get_hash();
   SHA_256::Hash get_fast_hash(int64_t start, int64_t size);

   std::string read_blob_data(Blob blob) final;
   Blob write_blob_data(const std::string &data) final;
 };
}

#endif
