#ifndef joedb_Client_Data_declared
#define joedb_Client_Data_declared

namespace joedb
{
 class Writable_Journal;
 class Readonly_Journal;
 class Readable;

 ////////////////////////////////////////////////////////////////////////////
 class Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual Writable_Journal &get_journal() = 0;
   virtual const Readonly_Journal &get_journal() const = 0;
   virtual void update() = 0;
   virtual ~Client_Data();
 };
}

#endif
