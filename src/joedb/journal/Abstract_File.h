#ifndef joedb_Abstract_File_declared
#define joedb_Abstract_File_declared

#include "joedb/journal/Open_Mode.h"
#include "joedb/Blob.h"

#include <stddef.h>
#include <stdint.h>
#include <string>

namespace joedb
{
 /// @ingroup journal
 class Abstract_File
 {
  private:
   Open_Mode mode;

  protected:
   void make_readonly() {mode = Open_Mode::read_existing;}
   void make_writable() {mode = Open_Mode::write_existing;}
   static constexpr int64_t last_position = (1ULL << 63) - 1;

  public:
   Abstract_File(Open_Mode mode):
    mode(mode)
   {
   }

   bool is_shared() const noexcept
   {
    return mode == Open_Mode::shared_write;
   }

   bool is_readonly() const noexcept
   {
    return mode == Open_Mode::read_existing;
   }

   Open_Mode get_mode() const noexcept
   {
    return mode;
   }

   /// Get the size of the file, or -1 if it is unknown
   virtual int64_t get_size() const {return -1;}

   /// Read a range of bytes
   ///
   /// The returned value may be less than size, even if the end of the file
   /// is not reached. 0 is returned if the end of the file is reached.
   virtual size_t pread(char *data, size_t size, int64_t offset) const {return 0;}

   /// Write a range of bytes. Extend file size if necessary.
   virtual void pwrite(const char *data, size_t size, int64_t offset) {}

   /// Write data durably (including file-size change)
   virtual void sync() {}

   /// Write data durably (no file-size change)
   virtual void datasync() {sync();}

   /// Lock a range of bytes for reading (prevents writes, not reads)
   virtual void shared_lock(int64_t start, int64_t size) {}

   /// Lock a range of bytes for writing (prevents both writes and reads)
   virtual void exclusive_lock(int64_t start, int64_t size) {}

   /// Remove a lock. The range should match the range of a corresponding lock
   virtual void unlock(int64_t start, int64_t size) noexcept {}

   void exclusive_lock_tail() {exclusive_lock(last_position, 1);}
   void unlock_tail() noexcept {unlock(last_position, 1);}

   void exclusive_lock_head() {exclusive_lock(0, 1);}
   void shared_lock_head() {shared_lock(0, 1);}
   void unlock_head() noexcept {unlock(0, 1);}

   class Head_Shared_Lock
   {
    private:
     Abstract_File &file;

    public:
     Head_Shared_Lock(Abstract_File &file): file(file)
     {
      file.shared_lock_head();
     }

     ~Head_Shared_Lock()
     {
      file.unlock_head();
     }
   };

   class Head_Exclusive_Lock
   {
    private:
     Abstract_File &file;

    public:
     Head_Exclusive_Lock(Abstract_File &file): file(file)
     {
      file.exclusive_lock_head();
     }
     ~Head_Exclusive_Lock()
     {
      file.unlock_head();
     }
   };

   std::string read_blob(Blob blob) const;

   static void reading_past_end_of_file();

   virtual void copy_to
   (
    Abstract_File &destination,
    int64_t start,
    int64_t size
   ) const;

   virtual bool equal_to
   (
    const Abstract_File &destination,
    int64_t from,
    int64_t until
   ) const;

   void copy_to(Abstract_File &destination) const
   {
    copy_to(destination, 0, get_size());
   }

   virtual ~Abstract_File() = default;
 };
}

#endif
