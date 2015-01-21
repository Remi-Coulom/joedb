#ifndef joedb_File_declared
#define joedb_File_declared

#include <cstdio>
#include <string>

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
   bool is_end_of_file() const {return std::feof(file) != 0;}
   mode_t get_mode() const {return mode;}

   void set_position(int64_t position);
   int64_t get_position() const;

   template<typename T> void write(T x) {W<T, sizeof(T)>::write(x, file);}
   template<typename T> T read() {return R<T, sizeof(T)>::read(file);}

   void write_string(const std::string &s);
   std::string read_string();

   void flush();

   ~File();

  private:
   FILE *file;
   mode_t mode;

   template<typename T, int n> struct W;
   template<typename T, int n> struct R;

   template<typename T>
   struct W<T, 1>
   {
    static void write(T x, FILE *file)
    {
     std::fputc(char(x), file);
    }
   };

   template<typename T>
   struct W<T, 2>
   {
    static void write(T x, FILE *file)
    {
     std::fputc(char(x >>  0), file);
     std::fputc(char(x >>  8), file);
    }
   };

   template<typename T>
   struct W<T, 4>
   {
    static void write(T x, FILE *file)
    {
     std::fputc(char(x >>  0), file);
     std::fputc(char(x >>  8), file);
     std::fputc(char(x >> 16), file);
     std::fputc(char(x >> 24), file);
    }
   };

   template<typename T>
   struct W<T, 8>
   {
    static void write(T x, FILE *file)
    {
     std::fputc(char(x >>  0), file);
     std::fputc(char(x >>  8), file);
     std::fputc(char(x >> 16), file);
     std::fputc(char(x >> 24), file);
     std::fputc(char(x >> 32), file);
     std::fputc(char(x >> 40), file);
     std::fputc(char(x >> 48), file);
     std::fputc(char(x >> 56), file);
    }
   };

   template<typename T>
   struct R<T, 1>
   {
    static T read(FILE *file)
    {
     return T(std::fgetc(file));
    }
   };

   template<typename T>
   struct R<T, 2>
   {
    static T read(FILE *file)
    {
     return T((uint16_t(std::fgetc(file)) <<  0) |
              (uint16_t(std::fgetc(file)) <<  8));
    }
   };

   template<typename T>
   struct R<T, 4>
   {
    static T read(FILE *file)
    {
     return T((uint32_t(std::fgetc(file)) <<  0) |
              (uint32_t(std::fgetc(file)) <<  8) |
              (uint32_t(std::fgetc(file)) << 16) |
              (uint32_t(std::fgetc(file)) << 24));
    }
   };

   template<typename T>
   struct R<T, 8>
   {
    static T read(FILE *file)
    {
     return T((uint64_t(std::fgetc(file)) <<  0) |
              (uint64_t(std::fgetc(file)) <<  8) |
              (uint64_t(std::fgetc(file)) << 16) |
              (uint64_t(std::fgetc(file)) << 24) |
              (uint64_t(std::fgetc(file)) << 32) |
              (uint64_t(std::fgetc(file)) << 40) |
              (uint64_t(std::fgetc(file)) << 48) |
              (uint64_t(std::fgetc(file)) << 56));
    }
   };
 };
}

#endif
