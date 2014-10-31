#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <iostream>

using namespace boost::property_tree;

/////////////////////////////////////////////////////////////////////////////
// Get C++ storage type for a db type
/////////////////////////////////////////////////////////////////////////////
std::string storage_type(const std::string &type)
{
 if (type == "string")
  return "std::string";
 else
  return type + "*";
}

/////////////////////////////////////////////////////////////////////////////
// Get C++ return type for a db type
/////////////////////////////////////////////////////////////////////////////
std::string return_type(const std::string &type)
{
 if (type == "string")
  return "const std::string&";
 else
  return type + "*";
}

/////////////////////////////////////////////////////////////////////////////
// write header
/////////////////////////////////////////////////////////////////////////////
void write_header(const ptree &pt, std::ostream &out)
{
 std::string dbname = pt.get<std::string>("database.name");

 out << "#ifndef " << dbname << "_declared\n";
 out << "#define " << dbname << "_declared\n";
 out << "\n#include <string>\n\n";
 out << "class " << dbname << "\n{\n";

 //
 // Loop over tables
 //
 bool first = true;
// for (const ptree::value_type &v: pt.get_child("database"))
 BOOST_FOREACH(const ptree::value_type &v, pt.get_child("database"))
  if (v.first == "table")
  {
   if (first)
    first = false;
   else
    out << '\n';

   std::string name = v.second.get<std::string>("name");
   out << " class " << name << "\n {\n  private:\n";

   //
   // Storage for each field
   //
   BOOST_FOREACH(const ptree::value_type &vv, v.second)
    if (vv.first == "field")
    {
     std::string name = vv.second.get<std::string>("name");
     std::string type = vv.second.get<std::string>("type");

     out << "   " << storage_type(type) << ' ' << name << ";\n";
    }

   //
   // Getter for each field
   //
   out << "\n  public:\n";
   BOOST_FOREACH(const ptree::value_type &vv, v.second)
    if (vv.first == "field")
    {
     std::string name = vv.second.get<std::string>("name");
     std::string type = vv.second.get<std::string>("type");

     out << "   " << return_type(type) << " get_" << name;
     out << "() const {return " << name << ";}\n";
    }

   //
   // Setter for each field
   //
   out << '\n';
   BOOST_FOREACH(const ptree::value_type &vv, v.second)
    if (vv.first == "field")
    {
     std::string name = vv.second.get<std::string>("name");
     std::string type = vv.second.get<std::string>("type");

     out << "   void set_" << name << '(' << return_type(type);
     out << " new_" << name << ");\n";
    }

   out << " };\n";
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
