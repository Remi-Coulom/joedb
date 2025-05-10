#ifndef joedb_Arguments_declared
#define joedb_Arguments_declared

#include <string_view>
#include <vector>
#include <ostream>

namespace joedb
{
 class Arguments
 {
  private:
   const int argc;
   const char * const * const argv;

   struct Argument
   {
    std::string_view s;
    std::string_view option;
    bool used;

    Argument(const char *argv);
   };

   std::vector<Argument> args;
   int index = 1;
   bool missing_arg = false;

   struct Option
   {
    std::string_view name;
    std::string_view parameter;
    const std::vector<const char *> *labels = nullptr;
    const size_t default_index = 0;

    Option(std::string_view parameter):
     parameter(parameter)
    {
    }

    Option(std::string_view name, std::string_view parameter):
     name(name),
     parameter(parameter)
    {
    }

    Option
    (
     std::string_view name,
     const std::vector<const char *> &labels,
     size_t default_index
    ):
     name(name),
     labels(&labels),
     default_index(default_index)
    {
    }
   };

   std::vector<Option> options;

   void update_index();
   std::string_view use_index();

  public:
   Arguments(int argc, const char * const *argv);

   bool has_option(const char * name);

   std::string_view get_string_option
   (
    const char * name,
    const char * description,
    std::string_view default_string
   );

   size_t get_enum_option
   (
    const char * name,
    const std::vector<const char *> &labels,
    size_t default_index
   );

   template<typename T> T get_option
   (
    const char * name,
    const char * description,
    T default_value
   );

   std::string_view get_next();
   std::string_view get_next(const char * parameter);
   void add_parameter(const char * parameter);
   std::ostream &print_help(std::ostream &out);

   int get_remaining_count() const {return int(args.size() - index);}
   int get_index() const {return int(index);}
   bool has_missing() const {return missing_arg;}
   size_t size() const {return args.size();}
   std::string_view operator[](size_t i) const {return args[i].s;}
   int get_argc() const {return argc;}
   const char * const *get_argv() const {return argv;}
   int &get_index() {return index;}
 };
}

#endif
