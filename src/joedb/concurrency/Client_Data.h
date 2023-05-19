#ifndef joedb_Client_Data_declared
#define joedb_Client_Data_declared

namespace joedb
{
 class Writable_Journal;

 ////////////////////////////////////////////////////////////////////////////
 class Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual Writable_Journal &get_journal() = 0;
   virtual void update() = 0;
   virtual ~Client_Data();
 };
}

#endif
