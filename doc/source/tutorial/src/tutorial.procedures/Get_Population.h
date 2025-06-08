#ifndef tutorial_procedures_Get_Population_declared
#define tutorial_procedures_Get_Population_declared

#include "../tutorial/procedures/population/Procedure.h"
#include "../tutorial/Database.h"

namespace tutorial::procedures
{
 class Get_Population: public population::Procedure
 {
  private:
   const tutorial::Database &db;

  public:
   Get_Population(tutorial::Database &db):
    population::Procedure("get_population"),
    db(db)
   {
   }

   void execute(population::Writable_Database &population) override
   {
    for (const auto data: population.get_data_table())
    {
     const std::string &city_name = population.get_city_name(data);
     const id_of_city city = db.find_city_by_name(city_name);

     int64_t count = 0;

     if (city.is_not_null())
      for (const auto person: db.get_person_table())
       if (db.get_home(person) == city)
        count++;

     population.set_city(data, city);
     population.set_count(data, count);
    }
   }
 };
}

#endif
