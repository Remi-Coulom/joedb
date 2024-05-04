#ifndef joedb_Generic_File_declared
#define joedb_Generic_File_declared

#include <string>
#include <cstdint>
#include <vector>

#include "joedb/assert.h"
#include "joedb/Blob.h"
#include "joedb/Posthumous_Thrower.h"
#include "joedb/journal/Open_Mode.h"
#include "joedb/journal/SHA_256.h"
#include "joedb/journal/Buffer.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Generic_File:
 ////////////////////////////////////////////////////////////////////////////
  public Posthumous_Thrower,
  public Blob_Reader,
  public Blob_Writer
 {
  friend class Async_Reader;
  friend class Async_Writer;

  private:
   int64_t slice_start;
   int64_t slice_length;

   Buffer buffer;

   int64_t file_position;
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

    read_buffer_size = raw_read(buffer.data, buffer.size);
    if (read_buffer_size == 0)
     end_of_file = true;

    file_position += read_buffer_size;
    buffer.index = 0;
   }

   //////////////////////////////////////////////////////////////////////////
   void write_buffer()
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_ASSERT(!buffer_has_read_data());

    raw_write(buffer.data, buffer.index);
    file_position += buffer.index;
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

   //////////////////////////////////////////////////////////////////////////
   void check_read_buffer()
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_ASSERT(!buffer_has_write_data());

    if (buffer.index >= read_buffer_size)
     read_buffer();
   }

   Open_Mode mode;
   const bool shared;
   bool locked_tail;

  protected:
   // ??? size_t should be int64_t ??? need 32-bit version testing.
   virtual size_t raw_read(char *data, size_t size) = 0;
   virtual void raw_write(const char *data, size_t size) = 0;
   virtual int raw_seek(int64_t offset) = 0; // 0 = OK, 1 = error
   virtual int64_t raw_get_size() const = 0; // -1 means no known size
   virtual void sync() = 0;

   int seek(int64_t offset)
   {
    return raw_seek(offset + slice_start);
   }

   size_t read_all(char *p, size_t capacity) // TODO -> cpp
   {
    size_t done = 0;
    while (done < capacity)
    {
     const size_t actually_read = raw_read(p + done, capacity - done);
     if (actually_read == 0)
      break;
     done += actually_read;
    }
    return done;
   };

   void destructor_flush() noexcept;

  public:
   virtual size_t raw_pread(char *data, size_t size, int64_t offset) // ->cpp
   {
    raw_seek(offset);
    return raw_read(data, size);
   }

   virtual void raw_pwrite(const char *data, size_t size, int64_t offset) // ->cpp
   {
    raw_seek(offset);
    raw_write(data, size);
   }

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
    slice_start = 0;
    slice_length = 0;

    file_position = 0;
    read_buffer_size = 0;
    end_of_file = false;
    buffer.index = 0;
   }

   //////////////////////////////////////////////////////////////////////////
   void flush()
   //////////////////////////////////////////////////////////////////////////
   {
    if (buffer_has_write_data())
     write_buffer();
    read_buffer_size = 0;
    buffer.index = 0;
   }

   //////////////////////////////////////////////////////////////////////////
   void commit()
   //////////////////////////////////////////////////////////////////////////
   {
    flush();
    sync();
   }

   //////////////////////////////////////////////////////////////////////////
   void set_slice(int64_t start, int64_t length) // -> cpp
   //////////////////////////////////////////////////////////////////////////
   {
    slice_start = start;
    slice_length = length;
    set_position(0);
   }

   //////////////////////////////////////////////////////////////////////////
   void set_mode(Open_Mode new_mode) // -> cpp
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

   //////////////////////////////////////////////////////////////////////////
   int64_t get_size() const
   //////////////////////////////////////////////////////////////////////////
   {
    if (slice_length)
     return slice_length;
    else
     return raw_get_size();
   }

   Open_Mode get_mode() const {return mode;}
   bool is_shared() const {return shared;}

   bool is_end_of_file() const {return end_of_file;}

   // set_position must be called when switching between write and read
   void set_position(int64_t position);

   int64_t get_position() const noexcept
   {
    return file_position - read_buffer_size + buffer.index;
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
    check_read_buffer();
    return buffer.compact_read<T>();
   }

   template<typename T> T read_strong_type()
   {
    return T(compact_read<typename std::underlying_type<T>::type>());
   }

   void write_reference(Record_Id id)
   {
    compact_write<>(to_underlying(id));
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
      raw_write(data, n);
      file_position += n;
     }
    }
   }

   //////////////////////////////////////////////////////////////////////////
   void read_data(char *data, const size_t n)
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_ASSERT(!buffer_has_write_data());

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
      const size_t actually_read = raw_read(data + n0, n - n0);
      if (actually_read == 0)
       break;

      file_position += actually_read;
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

   virtual ~Generic_File() = default; // ->cpp
 };
}

#endif
