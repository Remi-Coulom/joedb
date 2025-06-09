#ifndef tutorial_procedures_delete_city_declared
#define tutorial_procedures_delete_city_declared

#include "tutorial/procedures/city/Procedure.h"

namespace tutorial::procedures
{
 inline city::Write_Function delete_city =
 [](Writable_Database &db, city::Writable_Database &city)
 {
  const auto city_id = db.find_city_by_name(city.get_name());
  if (city_id.is_not_null())
   db.delete_city(city_id);
 };
}

#endif
