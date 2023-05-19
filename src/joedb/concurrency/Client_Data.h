#ifndef joedb_Client_Data_declared
#define joedb_Client_Data_declared

namespace joedb
{
 class Readonly_Journal;

 ////////////////////////////////////////////////////////////////////////////
 class Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual void update(Readonly_Journal &journal);
   virtual ~Client_Data();
 };
}

#endif
