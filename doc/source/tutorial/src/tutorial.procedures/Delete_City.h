#ifndef tutorial_procedures_Delete_City_declared
#define tutorial_procedures_Delete_City_declared

#include "tutorial/procedures/city/Procedure.h"

namespace tutorial::procedures
{
 class Delete_City: public city::Procedure
 {
  void execute(Client &client, city::Writable_Database &city) override
  {
   client.transaction([&city](Writable_Database &db)
   {
    const auto city_id = db.find_city_by_name(city.get_name());
    if (city_id.is_not_null())
     db.delete_city(city_id);
   });
  }
 };
}

#endif
