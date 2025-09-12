#ifndef joedb_Arguments_declared
#define joedb_Arguments_declared

#include <string_view>
#include <vector>
#include <ostream>
#include <sstream>

namespace joedb
{
 /// Class for conveniently parsing command-line arguments
 ///
 /// All strings passed as input to this class are zero-terminated, so all
 /// the std::string_view returned by member functions are zero-terminated.
 /// This way, it is safe to use view.data() as zero-terminated string.
 ///
 /// @ingroup ui
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

   bool has_flag(const char * name);

   std::string_view get_string_option
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
      std::istringstream(args[i + 1].s.data()) >> result;
      return result;
     }
    }
    return default_value;
   }

   template<typename T> T next_option
   (
    const char * name,
    const char * description,
    T default_value
   )
   {
    if (index < argc && args[index].option == name)
    {
     use_index();
     if (index < argc)
     {
      T result{};
      std::istringstream(args[index].s.data()) >> result;
      use_index();
      return result;
     }
    }

    return default_value;
   }

   std::string_view get_next();
   std::string_view get_next(const char * parameter);
   bool peek(const char *s);
   void add_parameter(const char * parameter);
   std::ostream &print_help(std::ostream &out) const;

   int get_remaining_count() const {return int(args.size() - index);}
   int get_index() const {return int(index);}
   bool missing() const {return missing_arg;}
   size_t size() const {return args.size();}
   std::string_view operator[](size_t i) const {return args[i].s;}
 };
}

#endif
