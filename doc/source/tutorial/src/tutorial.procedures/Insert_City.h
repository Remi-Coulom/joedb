#ifndef tutorial_procedures_Insert_City_declared
#define tutorial_procedures_Insert_City_declared

#include "tutorial/procedures/city/Procedure.h"
#include "tutorial/Client.h"

namespace tutorial::procedures
{
 class Insert_City: public city::Procedure
 {
  private:
   tutorial::Client &client;

  public:
   Insert_City(tutorial::Client &client): client(client)
   {
   }

   void execute(city::Writable_Database &city) override
   {
    client.transaction([&city](tutorial::Writable_Database &db)
    {
     db.new_city(city.get_name());
    });
   }
 };
}

#endif
