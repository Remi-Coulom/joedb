#include"all_includes.h"

namespace joedb
{
 typedef uint16_t Table_Id;
 typedef uint16_t Field_Id;
 typedef size_t Record_Id;
}

namespace joedb
{
 typedef uint8_t Type_Id_Storage;

 class Type
 {
  public:
   enum class Type_Id: uint8_t
   {
    null,
    string,
    int32,
    int64,
    reference,
    boolean,
    float32,
    float64,
    int8,
    int16,
   };

   enum {type_ids =
    1 +
    1 +
    1 +
    1 +
    1 +
    1 +
    1 +
    1 +
    1 +
    1
   };

  private:
   Type_Id type_id;
   Table_Id table_id;

   Type(Type_Id type_id,
        Table_Id table_id):
    type_id(type_id),
    table_id(table_id)
   {}

  public:
   Type_Id get_type_id() const {return type_id;}
   Table_Id get_table_id() const {return table_id;}

   Type(): type_id(Type_Id::null) {}
   Type(Type_Id type_id): type_id(type_id) {}

   static Type string() {return Type(Type_Id::string);};
   static Type int32() {return Type(Type_Id::int32);};
   static Type int64() {return Type(Type_Id::int64);};
   static Type boolean() {return Type(Type_Id::boolean);};
   static Type float32() {return Type(Type_Id::float32);};
   static Type float64() {return Type(Type_Id::float64);};
   static Type int8() {return Type(Type_Id::int8);};
   static Type int16() {return Type(Type_Id::int16);};

   static Type reference(Table_Id table_id)
   {
    return Type(Type_Id::reference, table_id);
   }
 };
}


namespace joedb
{
 enum class Commit_Level
 {
  no_commit,
  half_commit,
  full_commit
 };

 class Writable
 {
  public:
   virtual void create_table(const std::string &name) {}
   virtual void drop_table(Table_Id table_id) {}
   virtual void rename_table(Table_Id table_id, const std::string &name) {}

   virtual void add_field
   (
    Table_Id table_id,
    const std::string &name,
    Type type
   ) {}
   virtual void drop_field(Table_Id table_id, Field_Id field_id) {}
   virtual void rename_field
   (
    Table_Id table_id,
    Field_Id field_id,
    const std::string &name
   ) {}

   virtual void custom(const std::string &name) {}
   virtual void comment(const std::string &comment) {}
   virtual void timestamp(int64_t timestamp) {}
   virtual void valid_data() {}
   virtual void checkpoint(Commit_Level commit_level) {}

   virtual void insert_into(Table_Id table_id, Record_Id record_id) {}
   virtual void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    Record_Id size
   ) {}
   virtual void delete_from(Table_Id table_id, Record_Id record_id) {}


   virtual ~Writable() = default;
 };
}

namespace joedb
{

 class Runtime_Error: public std::runtime_error

 {
  public:
   explicit Runtime_Error(const char *what_arg):
    std::runtime_error(what_arg)
   {
   }

   explicit Runtime_Error(const std::string &what_arg):
    std::runtime_error(what_arg)
   {
   }
 };


 class Assertion_Failure: public Runtime_Error

 {
  public:
   explicit Assertion_Failure(const char *what_arg):
    Runtime_Error(what_arg)
   {
   }
 };


 class Exception: public Runtime_Error

 {
  public:
   explicit Exception(const char *what_arg):
    Runtime_Error(what_arg)
   {
   }

   explicit Exception(const std::string &what_arg):
    Runtime_Error(what_arg)
   {
   }
 };
}


namespace joedb
{

 inline void print_exception(const std::exception &e)

 {
  std::cout << "Error: " << e.what() << '\n';
 }


 inline int main_exception_catcher

 (
  int (*main)(int, char**), int argc, char **argv
 )
 {
  try
  {
   return main(argc, argv);
  }
  catch (const std::exception &e)
  {
   print_exception(e);
   return 1;
  }
 }
}










namespace joedb
{
 inline uint8_t is_big_endian()
 {
  const uint16_t n = 0x0100;
  return *(const uint8_t *)&n;
 }
}

















namespace joedb
{

 enum class Open_Mode

 {
  read_existing,
  write_existing,
  create_new,
  write_existing_or_create_new,
  shared_write
 };


 class Generic_File

 {
  friend class Async_Reader;
  friend class Async_Writer;

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
    do{ if (!(read_buffer_size == 0 && !end_of_file)) throw joedb::Assertion_Failure("!(""read_buffer_size == 0 && !end_of_file" ")\n  File: " "../../../../src/joedb/journal/Generic_File.h" "\n  Line: " "66");} while(0);
    buffer[write_buffer_index++] = c;
    position++;
   }


   char getc()

   {
    do{ if (!(write_buffer_index == 0)) throw joedb::Assertion_Failure("!(""write_buffer_index == 0" ")\n  File: " "../../../../src/joedb/journal/Generic_File.h" "\n  Line: " "75");} while(0);

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

   template<typename T, size_t n> struct R;


   template<typename T> struct R<T, 1>

   {
    static void change_endianness(T &x)
    {
    }

    static T read(Generic_File &file)
    {
     return T(file.getc());
    }
   };


   template<typename T> struct R<T, 2>

   {
    static void change_endianness(T &x)
    {
     char *p = reinterpret_cast<char *>(&x);
     std::swap(p[0], p[1]);
    }

    static T read(Generic_File &file)
    {
     T result;
     file.read_data(reinterpret_cast<char *>(&result), 2);
     if (is_big_endian())
      change_endianness(result);
     return result;
    }
   };


   template<typename T> struct R<T, 4>

   {
    static void change_endianness(T &x)
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
      change_endianness(result);
     return result;
    }
   };


   template<typename T> struct R<T, 8>

   {
    static void change_endianness(T &x)
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
      change_endianness(result);
     return result;
    }
   };

   template<typename T, size_t n> struct W;


   template<typename T> struct W<T, 1>

   {
    static void write(Generic_File &file, T x)
    {
     file.putc(char(x));
     file.check_write_buffer();
    }
   };


   template<typename T> struct W<T, 2>

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


   template<typename T> struct W<T, 4>

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


   template<typename T> struct W<T, 8>

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


   template<typename T> struct CW<T, 2>

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


   template<typename T> struct CW<T, 4>

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


   template<typename T> struct CW<T, 8>

   {
    static void write(Generic_File &file, T x)
    {
     if (!(uint64_t(x) >> 32))
      file.compact_write<uint32_t>(uint32_t(x));
     else
     {
      do{ if (!(!(char(x >> 56) & 0xe0))) throw joedb::Assertion_Failure("!(""!(char(x >> 56) & 0xe0)" ")\n  File: " "../../../../src/joedb/journal/Generic_File.h" "\n  Line: " "361");} while(0);
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
   const bool shared;

  protected:
   virtual size_t raw_read(char *buffer, size_t size) = 0;
   virtual void raw_write(const char *buffer, size_t size) = 0;
   virtual int raw_seek(int64_t offset) = 0;
   virtual int64_t raw_get_size() const = 0;
   virtual void sync() = 0;

   int seek(int64_t offset)
   {
    return raw_seek(offset + slice_start);
   }

   void destructor_flush() noexcept;

  public:

   Generic_File(Open_Mode mode):

    mode(mode),
    shared(mode == Open_Mode::shared_write)
   {
    write_buffer_index = 0;
    reset_read_buffer();
    position = 0;
    slice_start = 0;
    slice_length = 0;
   }


   void set_slice(int64_t start, int64_t length)

   {
    flush();
    slice_start = start;
    slice_length = length;
    set_position(0);
   }


   void set_mode(Open_Mode new_mode)

   {
    mode = new_mode;
   }


   int64_t get_size() const

   {
    if (slice_length)
     return slice_length;
    else
     return raw_get_size();
   }

   Open_Mode get_mode() const {return mode;}
   bool is_shared() const {return shared;}

   bool is_end_of_file() const {return end_of_file;}


   void set_position(int64_t position);
   int64_t get_position() const {return position;}
   void copy(Generic_File &source);


   template<typename T> void write(T x)

   {
    W<T, sizeof(T)>::write(*this, x);
   }


   template<typename T> T read()

   {
    return R<T, sizeof(T)>::read(*this);
   }


   template<typename T> static void change_endianness(T &x)

   {
    R<T, sizeof(T)>::change_endianness(x);
   }


   template<typename T> void compact_write(T x)

   {
    CW<T, sizeof(T)>::write(*this, x);
   }


   template<typename T> T compact_read()

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
    do{ if (!(write_buffer_index == 0)) throw joedb::Assertion_Failure("!(""write_buffer_index == 0" ")\n  File: " "../../../../src/joedb/journal/Generic_File.h" "\n  Line: " "500");} while(0);

    if (read_buffer_index + n <= read_buffer_size)
    {
     std::copy_n(buffer + read_buffer_index, n, data);
     read_buffer_index += n;
     position += n;
    }
    else
    {
     size_t n0 = read_buffer_size - read_buffer_index;
     std::copy_n(buffer + read_buffer_index, n0, data);
     read_buffer_index += n0;
     position += n0;

     if (n <= buffer_size)
     {
      read_buffer();

      while (n0 < n && read_buffer_index < read_buffer_size)
      {
       data[n0++] = buffer[read_buffer_index++];
       position++;
      }
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
     }

     if (n0 < n)
      end_of_file = true;
    }
   }

   void flush();
   void commit();

   virtual ~Generic_File() {}
 };
}


namespace joedb
{

 void Generic_File::set_position(int64_t new_position)

 {
  flush();
  if (!seek(new_position))
  {
   position = new_position;
   reset_read_buffer();
  }
 }


 void Generic_File::copy(Generic_File &source)

 {
  flush();
  source.set_position(0);

  while (true)
  {
   source.read_buffer();
   if (source.read_buffer_size == 0)
    break;
   std::copy_n(source.buffer, source.read_buffer_size, buffer);
   write_buffer_index = source.read_buffer_size;
   write_buffer();
   position += source.read_buffer_size;
  }
 }


 void Generic_File::write_string(const std::string &s)

 {
  compact_write<size_t>(s.size());
  for (char c: s)
   write<char>(c);
 }


 std::string Generic_File::read_string()

 {
  std::string s;
  size_t size = compact_read<size_t>();
  s.resize(size);
  read_data(&s[0], size);
  return s;
 }


 std::string Generic_File::safe_read_string(size_t max_size)

 {
  std::string s;
  size_t size = compact_read<size_t>();
  if (size < max_size)
  {
   s.resize(size);
   for (size_t i = 0; i < size; i++)
    s[i] = char(getc());
  }
  return s;
 }


 void Generic_File::flush()

 {
  if (write_buffer_index)
   write_buffer();
 }


 void Generic_File::destructor_flush() noexcept

 {
  if (write_buffer_index)
  {
   try
   {
    flush();
   }
   catch (...)
   {
   }
  }
 }


 void Generic_File::commit()

 {
  flush();
  sync();
 }
}





namespace joedb
{

 class Memory_File: public Generic_File

 {
  private:
   std::vector<char> data;
   size_t current;


   int64_t raw_get_size() const override

   {
    return int64_t(data.size());
   }


   size_t raw_read(char *buffer, size_t size) override

   {
    const size_t max_size = data.size() - current;
    const size_t n = std::min(size, max_size);
    std::copy_n(data.data() + current, n, buffer);
    current += n;
    return n;
   }


   void raw_write(const char *buffer, size_t size) override

   {
    const size_t end = current + size;
    if (end > data.size())
     data.resize(end);
    std::copy_n(buffer, size, &data[current]);
    current += size;
   }


   int raw_seek(int64_t offset) override

   {
    if (offset >= 0 && offset <= get_size())
    {
     current = size_t(offset);
     return 0;
    }
    else
     return 1;
   }


   void sync() override

   {
   }

  public:
   Memory_File(Open_Mode mode = Open_Mode::create_new);


   const std::vector<char> &get_data() const

   {
    return data;
   }

   ~Memory_File() override;
 };
}

namespace joedb
{

 Memory_File::Memory_File(Open_Mode mode):

  Generic_File(mode),
  current(0)
 {
 }


 Memory_File::~Memory_File()

 {
  destructor_flush();
 }
}





namespace joedb
{

 class Posix_File: public Generic_File

 {
  template<typename File_Type> friend class Local_Connection;

  private:
   int fd;

   void throw_last_error(const char *action, const char *file_name) const;

   bool try_lock();
   void lock();
   void unlock();

  protected:
   size_t raw_read(char *buffer, size_t size) override;
   void raw_write(const char *buffer, size_t size) override;
   int raw_seek(int64_t offset) override;
   void sync() override;

  public:
   Posix_File(int fd, Open_Mode mode):
    Generic_File(Open_Mode::read_existing),
    fd(fd)
   {
   }

   Posix_File(const char *file_name, Open_Mode mode);

   Posix_File(const std::string &file_name, Open_Mode mode):
    Posix_File(file_name.c_str(), mode)
   {
   }

   int64_t raw_get_size() const override;

   ~Posix_File() override;
 };
}



namespace joedb
{

 void Posix_File::throw_last_error

 (
  const char *action,
  const char *file_name
 ) const
 {
  throw Exception
  (
   std::string(action) + ' ' + file_name + ": " + strerror(errno)
  );
 }


 bool Posix_File::try_lock()

 {
  return flock(fd, LOCK_EX | LOCK_NB) == 0;
 }


 void Posix_File::lock()

 {
  if (flock(fd, LOCK_EX) == -1)
   throw_last_error("Locking", "file");
 }


 void Posix_File::unlock()

 {
  if (flock(fd, LOCK_UN) == -1)
   throw_last_error("Unlocking", "file");
 }


 size_t Posix_File::raw_read(char *buffer, size_t size)

 {
  const ssize_t result = ::read(fd, buffer, size);

  if (result < 0)
   throw_last_error("Reading", "file");

  return size_t(result);
 }


 void Posix_File::raw_write(const char *buffer, size_t size)

 {
  size_t written = 0;

  while (written < size)
  {
   const ssize_t result = ::write(fd, buffer + written, size - written);

   if (result < 0)
    throw_last_error("Writing", "file");
   else
    written += size_t(result);
  }
 }


 int Posix_File::raw_seek(int64_t offset)

 {
  return int64_t(lseek(fd, off_t(offset), SEEK_SET)) != offset;
 }


 void Posix_File::sync()

 {
  fsync(fd);
 }


 Posix_File::Posix_File(const char *file_name, const Open_Mode mode):

  Generic_File(mode)
 {
  if
  (
   mode == Open_Mode::write_existing_or_create_new ||
   mode == Open_Mode::shared_write
  )
  {
   fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, 00644);

   Open_Mode new_mode;

   if (fd >= 0)
    new_mode = Open_Mode::create_new;
   else
   {
    fd = open(file_name, O_RDWR);
    new_mode = Open_Mode::write_existing;
   }

   Generic_File::set_mode(new_mode);
  }
  else if (mode == Open_Mode::create_new)
   fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, 00644);
  else if (mode == Open_Mode::write_existing)
   fd = open(file_name, O_RDWR);
  else
   fd = open(file_name, O_RDONLY);

  if (fd < 0)
   throw_last_error("Opening", file_name);

  if (mode != Open_Mode::read_existing && mode != Open_Mode::shared_write)
  {
   if (!try_lock())
    throw_last_error("Locking", file_name);
  }
 }


 int64_t Posix_File::raw_get_size() const

 {
  struct stat s;

  if (fstat(fd, &s) == 0)
   return int64_t(s.st_size);
  else
   throw_last_error("Getting size of", "file");

  return -1;
 }


 Posix_File::~Posix_File()

 {
  if (fd >= 0)
  {
   destructor_flush();
   close(fd);
  }
 }
}









namespace joedb
{

 class Async_Reader

 {
  private:
   Generic_File &file;
   const int64_t initial_position;
   const int64_t end;
   int64_t current;

  public:

   Async_Reader(Generic_File &file, int64_t start, int64_t end):

    file(file),
    initial_position(file.get_position()),
    end(end),
    current(start)
   {
   }


   size_t read(char *buffer, size_t capacity)

   {
    size_t size = size_t(end - current);

    if (size > 0)
    {
     if (size > capacity)
      size = capacity;

     file.seek(current);
     file.raw_read(buffer, size);

     current += size;
    }

    return size;
   }


   int64_t get_remaining() const

   {
    return end - current;
   }


   ~Async_Reader()

   {
    file.seek(initial_position);
   }
 };
}


namespace joedb
{

 class Readonly_Journal

 {
  private:
   void read_checkpoint();

  protected:
   Generic_File &file;
   unsigned checkpoint_index;
   int64_t checkpoint_position;

   Table_Id table_of_last_operation;
   Record_Id record_of_last_operation;
   Field_Id field_of_last_update;

   Type read_type();
   std::string safe_read_string();









   enum class operation_t: uint8_t
   {
    end_of_file = 0x00,
    create_table = 0x01,
    drop_table = 0x02,
    add_field = 0x03,
    drop_field = 0x04,
    insert_into = 0x05,
    delete_from = 0x06,
    update = 0x07,
    append = 0x08,
    update_last = 0x09,
    comment = 0x0a,
    timestamp = 0x0b,
    rename_table = 0x0c,
    rename_field = 0x0d,
    valid_data = 0x0e,
    insert_vector = 0x0f,
    custom = 0x10,
    update_vector = 0x11,
    update_next = 0x12,
    updates = 0x80,










update_string, update_last_string, update_next_string, update_vector_string,


update_int32, update_last_int32, update_next_int32, update_vector_int32,
update_int64, update_last_int64, update_next_int64, update_vector_int64,


update_reference, update_last_reference, update_next_reference, update_vector_reference,


update_boolean, update_last_boolean, update_next_boolean, update_vector_boolean,


update_float32, update_last_float32, update_next_float32, update_vector_float32,
update_float64, update_last_float64, update_next_float64, update_vector_float64,


update_int8, update_last_int8, update_next_int8, update_vector_int8,
update_int16, update_last_int16, update_next_int16, update_vector_int16,
   };

  public:
   Readonly_Journal(Generic_File &file, bool ignore_errors = false);

   bool at_end_of_file() const;
   int64_t get_position() const {return file.get_position();}
   int64_t get_checkpoint_position() const {return checkpoint_position;}
   bool is_empty() const {return file.get_size() == header_size;}
   bool is_shared() const {return file.is_shared();}
   bool is_same_file(const Generic_File &other_file) const
   {
    return &file == &other_file;
   }

   void refresh_checkpoint();
   void replay_log(Writable &writable);
   void replay_with_checkpoint_comments(Writable &writable);
   void rewind();
   void set_position(int64_t position);
   void one_step(Writable &writable);
   void play_until(Writable &writable, int64_t end);
   void play_until_checkpoint(Writable &writable)
   {
    std::cerr << "A\n";
    play_until(writable, checkpoint_position);
    std::cerr << "B\n";
    writable.checkpoint(Commit_Level::no_commit);
    std::cerr << "C\n";
   }

   Async_Reader get_tail_reader(int64_t start_position) const
   {
    return Async_Reader(file, start_position, get_checkpoint_position());
   }

   std::vector<char> get_raw_tail(int64_t starting_position) const;

   static constexpr uint32_t version_number = 0x00000004;
   static constexpr uint32_t compatible_version = 0x00000004;
   static constexpr int64_t header_size = 41;
 };
}




constexpr uint32_t joedb::Readonly_Journal::version_number;
constexpr uint32_t joedb::Readonly_Journal::compatible_version;
constexpr int64_t joedb::Readonly_Journal::header_size;


joedb::Readonly_Journal::Readonly_Journal

(
 Generic_File &file,
 bool ignore_errors
):
 file(file),
 checkpoint_index(0),
 checkpoint_position(0),
 table_of_last_operation(0),
 record_of_last_operation(0),
 field_of_last_update(0)
{
 auto format_exception = [ignore_errors](const char *message)
 {
  if (!ignore_errors)
   throw Exception(message);
 };

 file.set_position(0);




 if (file.get_mode() != Open_Mode::create_new)
 {



  if (file.read<uint8_t>() != 'j' ||
      file.read<uint8_t>() != 'o' ||
      file.read<uint8_t>() != 'e' ||
      file.read<uint8_t>() != 'd' ||
      file.read<uint8_t>() != 'b')
  {
   format_exception("File does not start by 'joedb'");
  }
  else
  {



   const uint32_t version = file.read<uint32_t>();
   if (version < compatible_version || version > version_number)
    format_exception("Unsupported format version");

   read_checkpoint();




   const int64_t file_size = file.get_size();

   if (file_size > 0)
   {
    if (ignore_errors)
     checkpoint_position = file_size;
    else
    {
     if (file_size > checkpoint_position)
      throw Exception
      (
       "Checkpoint is smaller than file size. "
       "This file may contain an aborted transaction. "
       "joedb_convert can be used to fix it."
      );

     if (file_size < checkpoint_position)
      throw Exception("Checkpoint is bigger than file size");
    }
   }
  }
 }
}


void joedb::Readonly_Journal::read_checkpoint()

{
 checkpoint_position = header_size;

 int64_t pos[4];
 for (int i = 0; i < 4; i++)
  pos[i] = file.read<int64_t>();

 if (pos[0] != pos[1] || pos[2] != pos[3])
  throw Exception("Checkpoint mismatch");

 for (unsigned i = 0; i < 2; i++)
  if (pos[2 * i] == pos[2 * i + 1] && pos[2 * i] > checkpoint_position)
  {
   if (int64_t(size_t(pos[2 * i])) != pos[2 * i])
    throw Exception("size_t is too small for this file");
   checkpoint_position = pos[2 * i];
   checkpoint_index = i;
  }
}


void joedb::Readonly_Journal::refresh_checkpoint()

{
 const int64_t old_position = file.get_position();
 const int64_t checkpoint_offset = 5 + 4;
 file.set_position(checkpoint_offset);
 read_checkpoint();
 file.set_position(old_position);
}


std::vector<char> joedb::Readonly_Journal::get_raw_tail

(
 int64_t starting_position
) const
{
 std::vector<char> result;
 const int64_t size = get_checkpoint_position() - starting_position;

 if (size > 0)
 {
  result.resize(size_t(size));
  get_tail_reader(starting_position).read(result.data(), result.size());
 }

 return result;
}


void joedb::Readonly_Journal::replay_log(Writable &writable)

{
 rewind();
 play_until_checkpoint(writable);
}


void joedb::Readonly_Journal::replay_with_checkpoint_comments

(
 Writable &writable
)
{
 rewind();
 while(file.get_position() < checkpoint_position)
 {
  one_step(writable);
  writable.comment(std::to_string(file.get_position()));
 }
 writable.checkpoint(Commit_Level::no_commit);
}



void joedb::Readonly_Journal::rewind()

{
 file.set_position(header_size);
}


void joedb::Readonly_Journal::set_position(int64_t position)

{
 file.set_position(position);
}


void joedb::Readonly_Journal::play_until(Writable &writable, int64_t end)

{
 while(file.get_position() < end)
  one_step(writable);
 file.set_position(file.get_position());
}


bool joedb::Readonly_Journal::at_end_of_file() const

{
 return file.get_position() >= checkpoint_position;
}


void joedb::Readonly_Journal::one_step(Writable &writable)
{
}


joedb::Type joedb::Readonly_Journal::read_type()

{
 const Type::Type_Id type_id = Type::Type_Id(file.read<Type_Id_Storage>());
 if (type_id == Type::Type_Id::reference)
  return Type::reference(file.compact_read<Table_Id>());
 else
  return Type(type_id);
}


std::string joedb::Readonly_Journal::safe_read_string()

{
 return file.safe_read_string(size_t(checkpoint_position));
}

namespace joedb
{

 class Async_Writer

 {
  private:
   Generic_File &file;
   int64_t current;

  public:

   Async_Writer(Generic_File &file, int64_t start):

    file(file),
    current(start)
   {
   }


   void seek()

   {
    file.seek(current);
    file.position = current;
   }


   void write(const char *buffer, size_t size)

   {
    file.seek(current);
    file.raw_write(buffer, size);
    current += size;
    file.position = current;
   }
 };
}


namespace joedb
{

 class Writable_Journal:

  public Readonly_Journal,
  public Writable
 {
  public:
   Writable_Journal(Generic_File &file);


   class Tail_Writer

   {
    private:
     Writable_Journal &journal;
     const int64_t old_checkpoint;
     Async_Writer writer;

     Tail_Writer(const Tail_Writer &) = delete;

    public:
     Tail_Writer(Writable_Journal &journal):
      journal(journal),
      old_checkpoint(journal.get_checkpoint_position()),
      writer(journal.file, old_checkpoint)
     {
     }

     void append(const char *buffer, size_t size)
     {
      writer.write(buffer, size);
     }

     void finish()
     {
      writer.seek();
      journal.checkpoint(Commit_Level::no_commit);
      journal.file.set_position(old_checkpoint);
     }
   };

   void append_raw_tail(const char *data, size_t size);
   void append_raw_tail(const std::vector<char> &data);

   int64_t ahead_of_checkpoint() const;
   void checkpoint(Commit_Level commit_level) override;
   void flush() {file.flush();}

   void create_table(const std::string &name) override;
   void drop_table(Table_Id table_id) override;

   void rename_table
   (
    Table_Id table_id,
    const std::string &name
   ) override;

   void add_field
   (
    Table_Id table_id,
    const std::string &name,
    Type type
   ) override;

   void drop_field
   (
    Table_Id table_id,
    Field_Id field_id
   ) override;

   void rename_field
   (
    Table_Id table_id,
    Field_Id field_id,
    const std::string &name
   ) override;

   void custom(const std::string &name) override;
   void comment(const std::string &comment) override;
   void timestamp(int64_t timestamp) override;
   void valid_data() override;
   void insert_into(Table_Id table_id, Record_Id record_id) override;

   void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    Record_Id size
   ) override;

   void delete_from(Table_Id table_id, Record_Id record_id) override;






   ~Writable_Journal() override;
 };
}





joedb::Writable_Journal::Writable_Journal(Generic_File &file):

 Readonly_Journal(file)
{
 if (file.get_mode() == Open_Mode::create_new)
 {
  file.write<uint8_t>('j');
  file.write<uint8_t>('o');
  file.write<uint8_t>('e');
  file.write<uint8_t>('d');
  file.write<uint8_t>('b');
  file.write<uint32_t>(version_number);
  file.write<int64_t>(0);
  file.write<int64_t>(0);
  file.write<int64_t>(0);
  file.write<int64_t>(0);
  checkpoint(Commit_Level::no_commit);
 }
}


void joedb::Writable_Journal::append_raw_tail(const char *data, size_t size)

{
 Tail_Writer tail_writer(*this);
 tail_writer.append(data, size);
 tail_writer.finish();
}


void joedb::Writable_Journal::append_raw_tail(const std::vector<char> &data)

{
 append_raw_tail(data.data(), data.size());
}


int64_t joedb::Writable_Journal::ahead_of_checkpoint() const

{
 return file.get_position() - checkpoint_position;
}


void joedb::Writable_Journal::checkpoint(joedb::Commit_Level commit_level)

{
 std::cerr << "Writable_Journal::checkpoint\n";
}


void joedb::Writable_Journal::create_table(const std::string &name)

{
 file.write<operation_t>(operation_t::create_table);
 file.write_string(name);
}


void joedb::Writable_Journal::drop_table(Table_Id table_id)

{
 file.write<operation_t>(operation_t::drop_table);
 file.compact_write<Table_Id>(table_id);
}


void joedb::Writable_Journal::rename_table

(
 Table_Id table_id,
 const std::string &name
)
{
 file.write<operation_t>(operation_t::rename_table);
 file.compact_write<Table_Id>(table_id);
 file.write_string(name);
}


void joedb::Writable_Journal::add_field

(
 Table_Id table_id,
 const std::string &name,
 Type type
)
{
 file.write<operation_t>(operation_t::add_field);
 file.compact_write<Table_Id>(table_id);
 file.write_string(name);
 file.write<Type_Id_Storage>(Type_Id_Storage(type.get_type_id()));
 if (type.get_type_id() == Type::Type_Id::reference)
  file.compact_write<Table_Id>(type.get_table_id());
}


void joedb::Writable_Journal::drop_field

(
 Table_Id table_id,
 Field_Id field_id
)
{
 file.write<operation_t>(operation_t::drop_field);
 file.compact_write<Table_Id>(table_id);
 file.compact_write<Field_Id>(field_id);
}


void joedb::Writable_Journal::rename_field

(
 Table_Id table_id,
 Field_Id field_id,
 const std::string &name
)
{
 file.write<operation_t>(operation_t::rename_field);
 file.compact_write<Table_Id>(table_id);
 file.compact_write<Field_Id>(field_id);
 file.write_string(name);
}


void joedb::Writable_Journal::custom(const std::string &name)

{
 file.write<operation_t>(operation_t::custom);
 file.write_string(name);
}


void joedb::Writable_Journal::comment(const std::string &comment)

{
 file.write<operation_t>(operation_t::comment);
 file.write_string(comment);
}


void joedb::Writable_Journal::timestamp(int64_t timestamp)

{
 file.write<operation_t>(operation_t::timestamp);
 file.write<int64_t>(timestamp);
}


void joedb::Writable_Journal::valid_data()

{
 file.write<operation_t>(operation_t::valid_data);
}


void joedb::Writable_Journal::insert_into

(
 Table_Id table_id,
 Record_Id record_id
)
{
 if (table_id == table_of_last_operation &&
     record_id == record_of_last_operation + 1)
 {
  file.write<operation_t>(operation_t::append);
 }
 else
 {
  file.write<operation_t>(operation_t::insert_into);
  file.compact_write<Table_Id>(table_id);
  file.compact_write<Record_Id>(record_id);
 }

 table_of_last_operation = table_id;
 record_of_last_operation = record_id;
}


void joedb::Writable_Journal::insert_vector

(
 Table_Id table_id,
 Record_Id record_id,
 Record_Id size
)
{
 file.write<operation_t>(operation_t::insert_vector);
 file.compact_write<Table_Id>(table_id);
 file.compact_write<Record_Id>(record_id);
 file.compact_write<Record_Id>(size);

 table_of_last_operation = table_id;
 record_of_last_operation = record_id;
}


void joedb::Writable_Journal::delete_from

(
 Table_Id table_id,
 Record_Id record_id
)
{
 file.write<operation_t>(operation_t::delete_from);
 file.compact_write<Table_Id>(table_id);
 file.compact_write<Record_Id>(record_id);
}







joedb::Writable_Journal::~Writable_Journal()

{
}











namespace joedb
{

 class Mutex

 {
  private:
   virtual void lock() = 0;
   virtual void unlock() = 0;

   Mutex(const Mutex&) = delete;
   Mutex &operator=(const Mutex&) = delete;

  public:
   Mutex() {}
   virtual ~Mutex() = default;


   template<typename F> void run_while_locked(F f)

   {
    std::exception_ptr exception;

    lock();

    try
    {
     f();
    }
    catch (...)
    {
     exception = std::current_exception();
    }

    unlock();

    if (exception)
     std::rethrow_exception(exception);
   }
 };
}

namespace joedb
{

 class Connection: public Mutex

 {
  public:
   virtual int64_t handshake() = 0;

   virtual bool check_matching_content
   (
    Readonly_Journal &client_journal,
    int64_t checkpoint
   ) = 0;

   void lock() override = 0;
   void unlock() override = 0;

   virtual int64_t pull(Writable_Journal &client_journal) = 0;

   virtual int64_t lock_pull(Writable_Journal &client_journal)
   {
    lock();
    return pull(client_journal);
   }

   virtual int64_t lock_pull_unlock(Writable_Journal &client_journal)
   {
    int64_t result = lock_pull(client_journal);
    unlock();
    return result;
   }

   virtual void push
   (
    Readonly_Journal &client_journal,
    int64_t server_checkpoint,
    bool unlock_after
   ) = 0;

   virtual ~Connection() = default;

   Connection &locked(bool lock_me)
   {
    if (lock_me)
     lock();
    return *this;
   }
 };
}

namespace joedb
{

 template<typename Client_Data> class Client

 {
  private:
   void do_push(bool unlock_after)
   {
    connection.push(data.get_journal(), server_checkpoint, unlock_after);
    server_checkpoint = data.get_journal().get_checkpoint_position();
   }

   void push(bool unlock_after)
   {
    const int64_t difference = get_checkpoint_difference();

    if (difference < 0)
     throw Exception("can't push: server is ahead of client");

    if (difference > 0)
     do_push(unlock_after);
    else if (unlock_after)
     connection.unlock();
   }

   void throw_if_pull_when_ahead()
   {
    if (data.get_journal().get_position() > server_checkpoint)
     throw Exception("can't pull: client is ahead of server");
   }

  protected:
   Connection &connection;
   int64_t server_checkpoint;
   Client_Data data;

  public:
   Client
   (
    Connection &connection,
    Generic_File &file
   ):
    connection(connection),
    server_checkpoint(connection.handshake()),
    data(connection, file)
   {
    std::cerr << "data.update(), &data = " << &data << '\n';
    data.update();
   }

   int64_t get_checkpoint_difference() const

   {
    return data.get_journal().get_checkpoint_position() - server_checkpoint;
   }


   void locked_push()

   {
    push(false);
   }


   void push_unlock()

   {
    push(true);
   }


   int64_t pull()

   {
    throw_if_pull_when_ahead();

    if (data.get_journal().is_shared())
     server_checkpoint = connection.lock_pull_unlock(data.get_journal());
    else
     server_checkpoint = connection.pull(data.get_journal());

    data.update();
    return server_checkpoint;
   }


   template<typename F> void transaction(F transaction)

   {
    throw_if_pull_when_ahead();
    server_checkpoint = connection.lock_pull(data.get_journal());

    try
    {
     data.update();
     transaction();
     data.get_journal().checkpoint(Commit_Level::no_commit);
    }
    catch (...)
    {
     connection.unlock();
     data.get_journal().flush();
     throw;
    }

    push_unlock();
   }
 };
}






namespace joedb
{
 typedef Posix_File File;
}









namespace joedb
{
 std::string read_string(std::istream &in);
 void write_string(std::ostream &out, const std::string &s, bool json = false);
 void write_sql_string(std::ostream &out, const std::string &s);

 size_t utf8_display_size(const std::string &s);
 uint32_t read_utf8_char(size_t &i, const std::string &s);
 void write_justified
 (
  std::ostream &out,
  const std::string &s,
  size_t width,
  bool flush_left = true
 );

 char get_hex_char_from_digit(uint8_t n);
 uint8_t get_hex_digit_from_char(char c);
 void write_hexa_character(std::ostream &out, uint8_t c);
 void write_octal_character(std::ostream &out, uint8_t c);

 int8_t read_int8(std::istream &in);
 void write_int8(std::ostream &out, int8_t value);






 inline int32_t read_int32(std::istream &in) {int32_t value = int32_t(); in >> value; return value;} inline void write_int32(std::ostream &out, int32_t value) {out << value;}
 inline int64_t read_int64(std::istream &in) {int64_t value = int64_t(); in >> value; return value;} inline void write_int64(std::ostream &out, int64_t value) {out << value;}
 inline bool read_boolean(std::istream &in) {bool value = bool(); in >> value; return value;} inline void write_boolean(std::ostream &out, bool value) {out << value;}
 inline Record_Id read_reference(std::istream &in) {Record_Id value = Record_Id(); in >> value; return value;} inline void write_reference(std::ostream &out, Record_Id value) {out << value;}
 inline float read_float32(std::istream &in) {float value = float(); in >> value; return value;} inline void write_float32(std::ostream &out, float value) {out << value;}
 inline double read_float64(std::istream &in) {double value = double(); in >> value; return value;} inline void write_float64(std::ostream &out, double value) {out << value;}
 inline int16_t read_int16(std::istream &in) {int16_t value = int16_t(); in >> value; return value;} inline void write_int16(std::ostream &out, int16_t value) {out << value;}

}


namespace empty {
 using joedb::Record_Id;
 using joedb::Table_Id;
 using joedb::Field_Id;

 extern const char * schema_string;
 extern const size_t schema_string_size;

 class Database: public joedb::Writable
 {

  protected:
   virtual void error(const char *message)
   {
    throw joedb::Exception(message);
   }

   Record_Id max_record_id;

  public:
   void set_max_record_id(Record_Id record_id)
   {
    max_record_id = record_id;
   }

  protected:
   bool upgrading_schema = false;
   joedb::Memory_File schema_file;
   joedb::Writable_Journal schema_journal;

   bool requires_schema_upgrade() const
   {
    return schema_file.get_data().size() < schema_string_size;
   }

  public:
   Database():
    max_record_id(0),
    schema_journal(schema_file)
   {}
 };

 class Readonly_Database: public Database
 {
  public:
   Readonly_Database(joedb::Readonly_Journal &journal)
   {
    max_record_id = Record_Id(journal.get_checkpoint_position());
    journal.replay_log(*this);
    max_record_id = 0;

    if (requires_schema_upgrade())
     throw joedb::Exception
     (
      "Schema is out of date. Can't upgrade a read-only database."
     );
   }

   Readonly_Database(joedb::Readonly_Journal &&journal):
    Readonly_Database(journal)
   {
   }

   Readonly_Database(joedb::Generic_File &file):
    Readonly_Database(joedb::Readonly_Journal(file))
   {
   }

   Readonly_Database(joedb::Generic_File &&file):
    Readonly_Database(file)
   {
   }

   Readonly_Database(const char *file_name):
    Readonly_Database
    (
     joedb::File(file_name, joedb::Open_Mode::read_existing)
    )
   {
   }

   Readonly_Database(const std::string &file_name):
    Readonly_Database(file_name.c_str())
   {
   }
 };

 typedef Readonly_Database Generic_Readonly_Database;


 class Interpreted_File;
 class Interpreted_Database;


 class Readonly_Types

 {
  public:
   typedef empty::Database Database;
   typedef empty::Readonly_Database Readonly_Database;
   typedef empty::Interpreted_File Interpreted_File;
   typedef empty::Interpreted_Database Interpreted_Database;
 };
}
namespace empty {
 const char * schema_string = "joedb\004\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000)\000\000\000\000\000\000\000)\000\000\000\000\000\000\000";
 const size_t schema_string_size = 41;
}





namespace joedb
{

 template<typename File_Type> class Local_Connection: public Connection

 {
  private:
   File_Type file;

   int64_t handshake() override
   {
    int64_t result = 0;

    run_while_locked([&]()
    {
     Writable_Journal journal(file);
     result = journal.get_checkpoint_position();
    });

    return result;
   }

   void lock() final override
   {
    file.lock();
   }

   void unlock() final override
   {
    file.unlock();
   }

   int64_t pull(Writable_Journal &client_journal) override
   {
    throw Exception("Pulling into a shared file without locking");
   }

   int64_t lock_pull(Writable_Journal &client_journal) override
   {
    lock();
    client_journal.refresh_checkpoint();
    return client_journal.get_checkpoint_position();
   }

   int64_t lock_pull_unlock(Writable_Journal &client_journal) override
   {
    run_while_locked([&]()
    {
     client_journal.refresh_checkpoint();
    });
    return client_journal.get_checkpoint_position();
   }

   void push
   (
    Readonly_Journal &client_journal,
    int64_t server_position,
    bool unlock_after
   ) override
   {
    if (unlock_after)
     unlock();
   }

   bool check_matching_content
   (
    Readonly_Journal &client_journal,
    int64_t checkpoint
   ) override
   {
    return client_journal.is_same_file(file);
   }

  public:
   Local_Connection(const char *file_name):
    file(file_name, Open_Mode::shared_write)
   {
   }

   Local_Connection(const std::string &file_name):
    Local_Connection(file_name.c_str())
   {
   }

   File_Type &get_file() {return file;}
 };
}






namespace joedb
{

 template<typename T> class Span

 {
  private:
   T *p;
   size_t size;

  public:
   Span(T *p, size_t size): p(p), size(size)
   {
   }

   T &operator[](size_t i)
   {
    do{ if (!(i < size)) throw joedb::Assertion_Failure("!(""i < size" ")\n  File: " "../../../../src/joedb/Span.h" "\n  Line: " "25");} while(0);
    return p[i];
   }

   const T &operator[](size_t i) const
   {
    do{ if (!(i < size)) throw joedb::Assertion_Failure("!(""i < size" ")\n  File: " "../../../../src/joedb/Span.h" "\n  Line: " "31");} while(0);
    return p[i];
   }

   size_t get_size() const
   {
    return size;
   }

   T *begin()
   {
    return p;
   }

   T *end()
   {
    return p + size;
   }

   const T *begin() const
   {
    return p;
   }

   const T *end() const
   {
    return p + size;
   }
 };
}

namespace empty {

 class Client_Data;
 class Client;

 class Generic_File_Database: public Database
 {
  friend class Client_Data;
  friend class Client;

  protected:
   void error(const char *message) override
   {
    if (ready_to_write)
    {
     write_timestamp();
     write_comment(message);
     journal.flush();
    }
    Database::error(message);
   }

  private:
   joedb::Writable_Journal journal;
   bool ready_to_write;

   void initialize();
   void auto_upgrade();

   void custom(const std::string &name) override
   {
    Database::custom(name);
   }

   Generic_File_Database
   (
    joedb::Connection &connection,
    joedb::Generic_File &file
   );

  public:
   Generic_File_Database(joedb::Generic_File &file);

   int64_t ahead_of_checkpoint() const
   {
    return journal.ahead_of_checkpoint();
   }

   void checkpoint_no_commit()
   {
    journal.checkpoint(joedb::Commit_Level::no_commit);
   }

   void checkpoint_half_commit()
   {
    journal.checkpoint(joedb::Commit_Level::half_commit);
   }

   void checkpoint_full_commit()
   {
    journal.checkpoint(joedb::Commit_Level::full_commit);
   }

   void checkpoint()
   {
    checkpoint_no_commit();
   }

   void checkpoint(joedb::Commit_Level commit_level) override
   {
    journal.checkpoint(commit_level);
   }

   void write_comment(const std::string &comment);
   void write_timestamp();
   void write_timestamp(int64_t timestamp);
   void write_valid_data();
 };

 class File_Initialization
 {
  public:
   joedb::File file;

   File_Initialization(const char *file_name):
    file(file_name, joedb::Open_Mode::write_existing_or_create_new)
   {
   }
 };

 class File_Database:
  public File_Initialization,
  public Generic_File_Database
 {
  public:
   File_Database(const char *file_name):
    File_Initialization(file_name),
    Generic_File_Database(file)
   {
   }

   File_Database(const std::string &file_name):
    File_Database(file_name.c_str())
   {
   }
 };



 class Client_Data: public Generic_File_Database
 {
  public:
   Client_Data
   (
    joedb::Connection &connection,
    joedb::Generic_File &file
   ):
    Generic_File_Database(connection, file)
   {
    std::cerr << "\nClient_Data::Client_Data, this = " << this << '\n';
    update();
    std::cerr << '\n';
   }

   joedb::Writable_Journal &get_journal()
   {
    return journal;
   }

   const joedb::Writable_Journal &get_journal() const
   {
    return journal;
   }

   void update()
   {
    // Uncommenting the line below makes the crash go away
    // std::cerr << "inside Client_Data::update\n";
    journal.play_until_checkpoint(*static_cast<joedb::Writable *>(this));
   }
 };


 class Client: public joedb::Client<Client_Data>

 {
  private:
   int64_t schema_checkpoint;

   void throw_if_schema_changed()
   {
    if (data.schema_journal.get_checkpoint_position() > schema_checkpoint)
     throw joedb::Exception("Can't upgrade schema during pull");
   }

  public:
   Client
   (
    joedb::Connection &connection,
    joedb::Generic_File &local_file
   ):
    joedb::Client<Client_Data>(connection, local_file)
   {
#if 0
    if (get_checkpoint_difference() > 0)
     push_unlock();

    joedb::Client<Client_Data>::transaction([this](){
     data.auto_upgrade();
    });

    schema_checkpoint = data.schema_journal.get_checkpoint_position();
#endif
   }

   template<typename File> Client(joedb::Local_Connection<File> &connection):
    Client(connection, connection.get_file())
   {
   }

   const Database &get_database() const
   {
    return data;
   }

   int64_t pull()
   {
    const int64_t result = joedb::Client<Client_Data>::pull();
    throw_if_schema_changed();
    return result;
   }

   template<typename F> void transaction(F transaction)
   {
    joedb::Client<Client_Data>::transaction([&]()
    {
     throw_if_schema_changed();
     transaction(data);
    });
   }
 };



 class Local_Client_Parent

 {
  protected:
   joedb::Local_Connection<joedb::File> local_connection;

  public:
   Local_Client_Parent(const char *file_name):
    local_connection(file_name)
   {
   }
 };


 class Local_Client: public Local_Client_Parent, public Client

 {
  public:
   Local_Client(const char *file_name):
    Local_Client_Parent(file_name),
    Client(local_connection)
   {
   }

   Local_Client(const std::string &file_name):
    Local_Client(file_name.c_str())
   {
   }
 };



 class Types: public Readonly_Types

 {
  public:
   typedef empty::File_Database File_Database;
   typedef empty::Generic_File_Database Generic_File_Database;
   typedef empty::Client Client;
 };
}

namespace joedb
{

 class Readonly_Memory_File: public Generic_File

 {
  private:
   const char * const data;
   const size_t data_size;

   size_t current;


   int64_t raw_get_size() const override

   {
    return int64_t(data_size);
   }


   size_t raw_read(char *buffer, size_t size) override

   {
    const size_t max_size = data_size - current;
    const size_t n = std::min(size, max_size);
    std::copy_n(&data[current], n, buffer);
    current += n;
    return n;
   }


   void raw_write(const char *buffer, size_t size) override

   {
   }


   int raw_seek(int64_t offset) override

   {
    if (offset >= 0 && offset <= get_size())
    {
     current = size_t(offset);
     return 0;
    }
    else
     return 1;
   }

   void sync() override
   {
   }

  public:
   Readonly_Memory_File(const void *memory, size_t size):
    Generic_File(joedb::Open_Mode::read_existing),
    data((const char *)memory),
    data_size(size),
    current(0)
   {
   }
 };
}

namespace empty {
 void Generic_File_Database::write_comment(const std::string &comment)

 {
  journal.comment(comment);
 }


 void Generic_File_Database::write_timestamp()

 {
  journal.timestamp(std::time(nullptr));
 }

 void Generic_File_Database::write_timestamp(int64_t timestamp)
 {
  journal.timestamp(timestamp);
 }

 void Generic_File_Database::write_valid_data()
 {
  journal.valid_data();
 }

 void Generic_File_Database::initialize()
 {
  max_record_id = Record_Id(journal.get_checkpoint_position());
  ready_to_write = false;
  journal.replay_log(*this);
  ready_to_write = true;
  max_record_id = 0;
 }

 void Generic_File_Database::auto_upgrade()
 {
 }

 Generic_File_Database::Generic_File_Database

 (
  joedb::Connection &connection,
  joedb::Generic_File &file
 ):
  journal(file)
 {
  initialize();
 }

 Generic_File_Database::Generic_File_Database
 (
  joedb::Generic_File &file
 ):
  journal(file)
 {
  initialize();
  auto_upgrade();
 }
}

class Buggy_Client:
 private joedb::Local_Connection<joedb::Posix_File>,
 public empty::Client
{
 public:
  Buggy_Client(const char *file_name):
   joedb::Local_Connection<joedb::Posix_File>(file_name),
   empty::Client
   (
    *static_cast<joedb::Local_Connection<joedb::Posix_File>*>(this)
   )
  {
  }
};

static int local_concurrency(int argc, char **argv)
{
#if 1
 Buggy_Client client("local_concurrency.joedb");
#else
 joedb::Local_Connection<joedb::File> connection("local_concurrency.joedb");
 empty::Client client(connection);
#endif

 std::cerr << "Yeah, no bug!\n";
 return 0;
}

int main(int argc, char **argv)
{
// Not catching exceptions in main makes the bug go away
#if 1
 return joedb::main_exception_catcher(local_concurrency, argc, argv);
#else
 return local_concurrency(argc, argv);
#endif
}
