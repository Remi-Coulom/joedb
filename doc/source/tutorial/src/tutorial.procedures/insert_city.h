#ifndef tutorial_procedures_insert_city_declared
#define tutorial_procedures_insert_city_declared

#include "tutorial/procedures/city/Procedure.h"

namespace tutorial::procedures
{
 inline city::Write_Function insert_city =
 [](Writable_Database &db, city::Writable_Database &city)
 {
  db.new_city(city.get_name());
 };
}

#endif
