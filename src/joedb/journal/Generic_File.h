#ifndef joedb_Generic_File_declared
#define joedb_Generic_File_declared

#include "joedb/assert.h"
#include "joedb/Blob.h"
#include "joedb/Posthumous_Thrower.h"
#include "joedb/journal/Open_Mode.h"
#include "joedb/journal/SHA_256.h"
#include "joedb/journal/Buffer.h"

#include <string>
#include <cstdint>
#include <vector>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Abstract_File
 ////////////////////////////////////////////////////////////////////////////
 {
  // Should size be int64_t instead of size_t?
  // be careful when sizeof(size_t) == 4

  private:
   virtual size_t raw_read(char *data, size_t size)
   {
    return raw_pread(data, size, file_position);
   }

   virtual void raw_write(const char *data, size_t size)
   {
    raw_pwrite(data, size, file_position);
   }

   virtual size_t raw_pread(char *data, size_t size, int64_t offset)
   {
    raw_seek(offset);
    return raw_read(data, size);
   }

   virtual void raw_pwrite(const char *data, size_t size, int64_t offset)
   {
    raw_seek(offset);
    raw_write(data, size);
   }

   virtual void raw_seek(int64_t offset) = 0; // 0 = OK, 1 = error

   int64_t file_position;

  protected:
   int64_t slice_start;
   int64_t slice_length;

   int64_t get_file_position() const {return file_position;}

   size_t pos_read(char *data, size_t size)
   {
    size_t result = raw_read(data, size);
    file_position += result;
    return result;
   }

   void pos_write(const char *data, size_t size)
   {
    raw_write(data, size);
    file_position += size;
   }

   void seek(int64_t offset)
   {
    raw_seek(slice_start + offset);
    file_position = offset;
   }

   virtual int64_t raw_get_size() const {return -1;}
   virtual void raw_sync() {}

  public:
   Abstract_File()
   {
    slice_start = 0;
    slice_length = 0;
    file_position = 0;
   }

   // Note: file_position is undefined after those

   size_t pos_pread(char *data, size_t size, int64_t offset)
   {
    return raw_pread(data, size, slice_start + offset);
   }

   void pos_pwrite(const char *data, size_t size, int64_t offset)
   {
    raw_pwrite(data, size, slice_start + offset);
   }

   int64_t get_size() const
   {
    if (slice_start + slice_length)
     return slice_length;
    else
     return raw_get_size();
   }

   virtual ~Abstract_File() = default;
 };

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
   size_t read_all(char *p, size_t capacity)
   {
    size_t done = 0;
    while (done < capacity)
    {
     const size_t actually_read = pos_read(p + done, capacity - done);
     if (actually_read == 0)
      break;
     done += actually_read;
    }
    return done;
   };

   void destructor_flush() noexcept;

  public:
   //////////////////////////////////////////////////////////////////////////
   Generic_File(Open_Mode mode):
   //////////////////////////////////////////////////////////////////////////
    mode(mode),
    shared(mode == Open_Mode::shared_write),
    locked_tail
    (
     mode != Open_Mode::shared_write &&
     mode != Open_Mode::read_existing
    ) // ->cpp
   {
    read_buffer_size = 0;
    end_of_file = false;
    buffer.index = 0;
   }

   void flush();

   //////////////////////////////////////////////////////////////////////////
   void commit()
   //////////////////////////////////////////////////////////////////////////
   {
    flush();
    raw_sync();
   }

   //////////////////////////////////////////////////////////////////////////
   void set_slice(int64_t start, int64_t length)
   //////////////////////////////////////////////////////////////////////////
   {
    slice_start = start;
    slice_length = length;
    set_position(0);
   }

   //////////////////////////////////////////////////////////////////////////
   void set_mode(Open_Mode new_mode)
   //////////////////////////////////////////////////////////////////////////
   {
    mode = new_mode;
   }

   virtual void shared_lock(int64_t start, int64_t size);
   virtual void exclusive_lock(int64_t start, int64_t size);
   virtual void unlock(int64_t start, int64_t size);

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

   void copy(Generic_File &source, int64_t start, int64_t size);

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
   void read_data(char *data, const size_t n)
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_ASSERT(!buffer_has_write_data());
    JOEDB_ASSERT(buffer.index <= read_buffer_size);

    if (buffer.index + n <= read_buffer_size)
    {
     std::copy_n(buffer.data + buffer.index, n, data);
     buffer.index += n;
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
       break;
      n0 += actually_read;
     }

     if (n0 < n)
      end_of_file = true;
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
