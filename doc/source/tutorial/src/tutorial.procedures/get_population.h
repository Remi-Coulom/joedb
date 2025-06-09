#ifndef tutorial_procedures_get_population_declared
#define tutorial_procedures_get_population_declared

#include "tutorial/procedures/population/Procedure.h"

namespace tutorial::procedures
{
 inline population::Read_Function get_population =
 [](const tutorial::Database &db, population::Writable_Database &population)
 {
  for (const auto data: population.get_data_table())
  {
   const std::string &city_name = population.get_city_name(data);
   const id_of_city city = db.find_city_by_name(city_name);

   int64_t N = 0;

   if (city.is_not_null())
    for (const auto person: db.get_person_table())
     if (db.get_home(person) == city)
      N++;

   population.set_city(data, city);
   population.set_population(data, N);
  }
 };
}

#endif
