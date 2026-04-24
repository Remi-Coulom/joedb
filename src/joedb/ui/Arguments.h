#ifndef joedb_Arguments_declared
#define joedb_Arguments_declared

#include "external/cstring_view.hpp"

#include <vector>
#include <ostream>
#include <sstream>

namespace joedb
{
 /// Class for conveniently parsing command-line arguments
 ///
 /// @ingroup ui
 class Arguments
 {
  private:
   const int argc;
   const char * const * const argv;

   struct Argument
   {
    beman::cstring_view s;
    beman::cstring_view option;
    bool used;

    Argument(const char *argv);
   };

   std::vector<Argument> args;
   int index = 1;
   bool missing_arg = false;

   struct Option
   {
    beman::cstring_view name;
    beman::cstring_view parameter;
    const std::vector<const char *> *labels = nullptr;
    const size_t default_index = 0;

    Option(beman::cstring_view parameter):
     parameter(parameter)
    {
    }

    Option(beman::cstring_view name, beman::cstring_view parameter):
     name(name),
     parameter(parameter)
    {
    }

    Option
    (
     beman::cstring_view name,
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
   beman::cstring_view use_index();

  public:
   Arguments(int argc, const char * const *argv);
   Arguments(): Arguments(0, nullptr) {missing_arg = true;}

   bool has_flag(const char * name);

   beman::cstring_view get_string_option
   (
    const char * name,
    const char * description,
    const char * default_string
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
   )
   {
    options.emplace_back(name, description);

    for (size_t i = 0; i < args.size() - 1; i++)
    {
     if (args[i].option == name)
     {
      args[i].used = true;
      args[i + 1].used = true;
      update_index();
      T result{};

      if constexpr (std::is_same<T, std::string>::value || std::is_same<T, beman::cstring_view>::value)
       result = args[i + 1].s;
      else
       std::istringstream(args[i + 1].s.c_str()) >> result;

      return result;
     }
    }
    return default_value;
   }

   template<typename T = beman::cstring_view> T next_option
   (
    const char * name,
    const char * description,
    T default_value
   )
   {
    options.emplace_back(name, description);

    if (index < argc && args[index].option == name)
    {
     use_index();
     if (index < argc)
     {
      T result{};

      if constexpr (std::is_same<T, std::string>::value || std::is_same<T, beman::cstring_view>::value)
       result = args[index].s;
      else
       std::istringstream(args[index].s.c_str()) >> result;

      use_index();
      return result;
     }
    }

    return default_value;
   }

   beman::cstring_view get_next();
   beman::cstring_view get_next(const char * parameter);
   bool peek(beman::cstring_view s);
   void add_parameter(const char * parameter);
   std::ostream &print_help(std::ostream &out) const;
   bool has_error() const {return missing() || get_remaining_count();}

   int get_remaining_count() const {return int(args.size() - index);}
   int get_index() const {return int(index);}
   bool missing() const {return missing_arg;}
   size_t size() const {return args.size();}
   beman::cstring_view operator[](size_t i) const {return args[i].s;}
 };
}

#endif
