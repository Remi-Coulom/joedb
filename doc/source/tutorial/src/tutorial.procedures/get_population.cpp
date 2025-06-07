#include "../tutorial/procedures/get_population.h"

namespace tutorial::procedures
{
 void execute
 (
  Client &client,
  get_population::Memory_Database &get_population
 )
 {
  const Database &db = client.get_database();

  for (const auto data: get_population.get_data_table())
  {
   const std::string &city_name = get_population.get_city_name(data);
   const id_of_city city = db.find_city_by_name(city_name);

   int64_t population = 0;

   if (city.is_not_null())
    for (const auto person: db.get_person_table())
     if (db.get_home(person) == city)
      population++;

   get_population.set_population(data, population);
   get_population.set_city(data, city);
  }
 }
}
