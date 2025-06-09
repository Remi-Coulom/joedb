#ifndef tutorial_procedures_Insert_City_declared
#define tutorial_procedures_Insert_City_declared

#include "tutorial/procedures/city/Procedure.h"

namespace tutorial::procedures
{
 class Insert_City: public city::Procedure
 {
  void execute(Client &client, city::Writable_Database &city) override
  {
   client.transaction([&city](Writable_Database &db)
   {
    db.new_city(city.get_name());
   });
  }
 };
}

#endif
