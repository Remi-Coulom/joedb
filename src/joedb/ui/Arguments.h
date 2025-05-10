#ifndef joedb_Arguments_declared
#define joedb_Arguments_declared

#include "joedb/error/Exception.h"
#include "joedb/get_version.h"

#include <string_view>
#include <vector>
#include <charconv>
#include <ostream>

namespace joedb
{
 class Arguments
 {
  private:
   struct Argument
   {
    std::string_view s;
    std::string_view option;
    bool used;

    Argument(const char *argv): s(argv), used(false)
    {
     if (s.size() > 2 && s[0] == '-' && s[1] == '-')
      option = std::string_view(s.data() + 2, s.size() - 2);
    }
   };

   std::vector<Argument> args;
   size_t index = 1;
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

   void update_index()
   {
    while (index < args.size() && args[index].used)
     index++;
   }

  public:
   Arguments(size_t argc, const char * const *argv)
   {
    args.reserve(argc);
    for (size_t i = 0; i < argc; i++)
     args.emplace_back(argv[i]);
   }

   bool has_option(std::string_view name)
   {
    options.emplace_back(name, std::string_view{});

    for (auto &arg: args)
    {
     if (arg.option == name)
     {
      arg.used = true;
      update_index();
      return true;
     }
    }

    return false;
   }

   std::string_view get_string_option
   (
    std::string_view name,
    std::string_view description,
    std::string_view default_string
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
      return args[i + 1].s;
     }
    }

    return default_string;
   }

   size_t get_enum_option
   (
    std::string_view name,
    const std::vector<const char *> &labels,
    size_t default_index
   )
   {
    options.emplace_back(name, labels, default_index);

    for (size_t i = 0; i < args.size() - 1; i++)
    {
     if (args[i].option == name)
     {
      args[i].used = true;
      args[i + 1].used = true;
      update_index();

      for (int j = int(labels.size()); --j >= 0;)
       if (labels[j] == args[i + 1].s)
        return size_t(j);

      throw Exception
      (
       std::string("invalid value for option ") + std::string(args[i].s) +
       ": " + std::string(args[i + 1].s)
      );
     }
    }

    return default_index;
   }

   template<typename T>
   T get_option
   (
    std::string_view name,
    std::string_view description,
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
      T result;
      if
      (
       std::from_chars
       (
        args[i + 1].s.data(),
        args[i + 1].s.data() + args[i +1].s.size(),
        result
       )
       .ec != std::errc{}
      )
      {
       throw Exception
       (
        std::string("Error parsing option value for ") + std::string(args[i].s)
       );
      }
      return result;
     }
    }
    return default_value;
   }

   std::string_view get_next()
   {
    if (index < args.size())
     return args[index++].s;
    else
     return std::string_view{};
   }

   void add_parameter(const std::string_view parameter)
   {
    options.emplace_back(parameter);
   }

   std::string_view get_next_arg(const std::string_view parameter)
   {
    options.emplace_back(parameter);

    if (index < args.size())
    {
     args[index].used = true;
     const auto result = args[index].s;
     update_index();
     return result;
    }
    else
    {
     missing_arg = true;
     return std::string_view{};
    }
   }

   int get_remaining_count() const
   {
    return int(args.size() - index);
   }

   int get_index() const
   {
    return int(index);
   }

   bool has_missing() const
   {
    return missing_arg;
   }

   std::ostream &print_help(std::ostream &out)
   {
    out << "joedb version: " << get_version() << '\n';
    out << "usage: " << args[0].s;

    for (const auto &option: options)
    {
     if (option.name.data())
     {
      out << " [--";
      out << option.name;
      if (option.parameter.data())
       out << " <" << option.parameter << '>';
      else if (option.labels)
      {
       for (size_t i = 0; i < option.labels->size(); i++)
       {
        if (i > 0)
         out << '|';
        out << option.labels->at(i);
        if (size_t(option.default_index) == i)
         out << '*';
       }
      }
      out << ']';
     }
     else if (option.parameter.data())
      out << " <" << option.parameter << '>';
    }

    out << '\n';

    return out;
   }

   std::string_view operator[](size_t i) const {return args[i].s;}
 };
}

#endif
