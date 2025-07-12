#ifndef tutorial_procedures_Service_declared
#define tutorial_procedures_Service_declared

#include "joedb/Thread_Safe.h"
#include "tutorial/Client.h"
#include "tutorial/procedures/city/Writable_Database.h"
#include "tutorial/procedures/population/Writable_Database.h"

namespace tutorial::procedures
{
 class Service
 {
  private:
   joedb::Thread_Safe<Client&> client;

  public:
   Service(Client &client): client(client)
   {
   }

   void insert_city(city::Writable_Database &city) 
   {
    joedb::Lock<Client&>(client)->transaction
    (
     [&city](Writable_Database &db)
     {
      db.new_city(city.get_name());
     }
    );
   }

   void delete_city(city::Writable_Database &city) 
   {
    joedb::Lock<Client&>(client)->transaction
    (
     [&city](Writable_Database &db)
     {
      const auto city_id = db.find_city_by_name(city.get_name());
      if (city_id.is_not_null())
       db.delete_city(city_id);
     }
    );
   }

   void get_population(population::Writable_Database &population)
   {
    joedb::Lock<Client&> lock(client);
    lock->pull();
    const auto &db = lock->get_database();

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
   }
 };
}

#endif
