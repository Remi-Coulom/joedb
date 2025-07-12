#ifndef tutorial_rpc_Service_declared
#define tutorial_rpc_Service_declared

#include "tutorial/Client.h"
#include "tutorial/rpc/city/Writable_Database.h"
#include "tutorial/rpc/population/Writable_Database.h"

namespace tutorial::rpc
{
 /// A collection of procedures that will be executed in the rpc server
 class Service
 {
  private:
   Client &client;

  public:
   Service(Client &client): client(client)
   {
   }

   void insert_city(city::Writable_Database &city) 
   {
    client.transaction
    (
     [&city](Writable_Database &db)
     {
      db.new_city(city.get_name());
     }
    );
   }

   void delete_city(city::Writable_Database &city) 
   {
    client.transaction
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
    client.pull();
    const auto &db = client.get_database();

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
