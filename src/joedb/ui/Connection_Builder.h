#ifndef joedb_Connection_Builder_declared
#define joedb_Connection_Builder_declared

namespace joedb::concurrency
{
 class Pullonly_Connection;
}

namespace joedb
{
 class Buffered_File;
}

namespace joedb::ui
{
 ////////////////////////////////////////////////////////////////////////////
 class Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual bool has_sharing_option() const {return false;}
   virtual bool get_default_sharing() const {return false;}
   virtual int get_min_parameters() const {return 0;}
   virtual int get_max_parameters() const {return 0;}
   virtual const char *get_name() const {return "";}
   virtual const char *get_parameters_description() const {return "";}

   virtual concurrency::Pullonly_Connection &build(int argc, char **argv, Buffered_File *file) = 0;

   virtual ~Connection_Builder() = default;
 };
}

#endif
