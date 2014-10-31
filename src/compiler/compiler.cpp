#include <boost/property_tree/xml_parser.hpp>
#include <iostream>

using namespace boost::property_tree;

const char * const id_type = "uint32_t";

/////////////////////////////////////////////////////////////////////////////
// Get C++ storage type for a db type
/////////////////////////////////////////////////////////////////////////////
std::string storage_type(const std::string &type)
{
 if (type == "string")
  return "std::string";
 else
  return id_type;
}

/////////////////////////////////////////////////////////////////////////////
// Get C++ return type for a db type
/////////////////////////////////////////////////////////////////////////////
std::string return_type(const std::string &type)
{
 if (type == "string")
  return "const std::string&";
 else
  return id_type;
}

/////////////////////////////////////////////////////////////////////////////
// write header
/////////////////////////////////////////////////////////////////////////////
void write_header(const ptree &pt, std::ostream &out)
{
 std::string dbname = pt.get<std::string>("database.name");

 out << "#ifndef " << dbname << "_declared\n";
 out << "#define " << dbname << "_declared\n";
 out << '\n';
 out << "#include <string>\n";
 out << "#include <vector>\n";
 out << '\n';
 out << "class " << dbname << "\n{\n";

 //
 // Class definition of each table
 //
 out << " public:\n";
 for (const auto &v: pt.get_child("database"))
  if (v.first == "table")
  {
   std::string name = v.second.get<std::string>("name");
   out << "  class " << name << '\n';
   out << "  {\n";
   out << "   private:\n";
   out << "    " << id_type << " id;\n";

   //
   // Storage for each field
   //
   for (const auto &vv: v.second)
    if (vv.first == "field")
    {
     std::string name = vv.second.get<std::string>("name");
     std::string type = vv.second.get<std::string>("type");

     out << "    " << storage_type(type) << ' ' << name << ";\n";
    }

   //
   // Getter for each field
   //
   out << '\n';
   out << "   public:\n";
   out << "    " << id_type << " get_id() const {return id;}\n";
   for (const auto &vv: v.second)
    if (vv.first == "field")
    {
     std::string name = vv.second.get<std::string>("name");
     std::string type = vv.second.get<std::string>("type");

     out << "    " << return_type(type) << " get_" << name;
     out << "() const {return " << name << ";}\n";
    }

   //
   // Setter for each field
   //
   out << '\n';
   for (const auto &vv: v.second)
    if (vv.first == "field")
    {
     std::string name = vv.second.get<std::string>("name");
     std::string type = vv.second.get<std::string>("type");

     out << "    void set_" << name << '(' << return_type(type);
     out << " new_" << name << ");\n";
    }

   out << "    void destroy() {id = 0;}\n";

   out << "  };\n\n";
  }

 //
 // Private data
 //
 out << " private:\n";
 for (const auto &v: pt.get_child("database"))
  if (v.first == "table")
  {
   std::string name = v.second.get<std::string>("name");
   out << "  std::vector<" << name << "> " << name << "_storage;\n";
  }

 //
 // Public interface
 //
 out << '\n';
 out << " public:\n";
 for (const auto &v: pt.get_child("database"))
  if (v.first == "table")
  {
   std::string name = v.second.get<std::string>("name");
   std::string storage = name + "_storage";

   out << "  " << name << "& new_" << name << "();\n";

   out << "  " << name << "& get_" << name;
   out << "(" << id_type << " id) {return ";
   out << storage << "[id - 1];}\n";

   out << "  const " << name << "& get_" << name;
   out << "(" << id_type << " id) const {return ";
   out << storage << "[id - 1];}\n";

   out << "  " << id_type << " next_" << name;
   out << '(' << id_type << " id) const\n";
   out << "  {\n";
   out << "   while (true)\n";
   out << "   {\n";
   out << "    if (id >= " << storage << ".size())\n";
   out << "     return 0;\n";
   out << "    id = id + 1;\n";
   out << "    if (" << storage << "[id - 1].get_id() == id)\n";
   out << "     return id;\n";
   out << "   }\n";
   out << "  }\n";

   out << '\n';
  }

 out << "};\n\n";
 out << "#endif\n";
}

/////////////////////////////////////////////////////////////////////////////
// main
/////////////////////////////////////////////////////////////////////////////
int main()
{
 ptree pt;
 xml_parser::read_xml(std::cin, pt);
 write_header(pt, std::cout);

 return 0;
}
