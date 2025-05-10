#include "joedb/ui/Arguments.h"
#include "joedb/error/assert.h"

#include <charconv>

namespace joedb
{
 Arguments::Argument::Argument(const char *argv): s(argv), used(false)
 {
  if (s.size() > 2 && s[0] == '-' && s[1] == '-')
   option = std::string_view(s.data() + 2, s.size() - 2);
 }

 void Arguments::update_index()
 {
  while (index < argc && args[index].used)
   index++;
 }

 std::string_view Arguments::use_index()
 {
  JOEDB_DEBUG_ASSERT(index < argc);
  args[index].used = true;
  const auto result = args[index].s;
  update_index();
  return result;
 }

 Arguments::Arguments(int argc, const char * const *argv):
  argc(argc),
  argv(argv)
 {
  args.reserve(argc);
  for (int i = 0; i < argc; i++)
   args.emplace_back(argv[i]);
 }

 bool Arguments::has_option(const char * name)
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

 std::string_view Arguments::get_string_option
 (
  const char * name,
  const char * description,
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

 size_t Arguments::get_enum_option
 (
  const char * name,
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
 T Arguments::get_option
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

 template int64_t Arguments::get_option<int64_t>
 (
  const char * name,
  const char * description,
  int64_t default_value
 );

 template float Arguments::get_option<float>
 (
  const char * name,
  const char * description,
  float default_value
 );

 std::string_view Arguments::get_next()
 {
  if (index < argc)
   return use_index();
  else
   return std::string_view{};
 }

 void Arguments::add_parameter(const char * parameter)
 {
  options.emplace_back(parameter);
 }

 std::string_view Arguments::get_next(const char * parameter)
 {
  options.emplace_back(parameter);

  if (index < argc)
   return use_index();
  else
  {
   missing_arg = true;
   return std::string_view{};
  }
 }

 std::ostream &Arguments::print_help(std::ostream &out)
 {
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
      out << (i > 0 ? '|' : ' ');
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
}
