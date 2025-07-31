#ifndef tutorial_rpc_Service_declared
#define tutorial_rpc_Service_declared

#include "tutorial/Client.h"
#include "tutorial/rpc/city/Writable_Database.h"
#include "tutorial/rpc/population/Writable_Database.h"

namespace tutorial::rpc
{
 /// A collection of procedures that will be executed in the rpc server
 ///
 /// joedbc uses a regular expression to find procedures in the file.
 /// The constructor is not called from compiled code, so it can have any
 /// signature. In particular, a Service does not necessarily have to
 /// access a database at all.
 class Service
 {
  private:
   Client &client;

  public:
   Service(Client &client): client(client)
   {
   }

   /// Insert a city from a name string
   void insert_city(city::Writable_Database &city) 
   {
    client.transaction
    (
     [&city](Writable_Database &db)
     {
      const auto city_id = db.find_city_by_name(city.get_name());
      if (city_id.is_null())
       db.new_city(city.get_name());
      else
       throw joedb::Exception("city already exists");
     }
    );
   }

   /// Delete a city from a name string
   void delete_city(city::Writable_Database &city) 
   {
    client.transaction
    (
     [&city](Writable_Database &db)
     {
      const auto city_id = db.find_city_by_name(city.get_name());
      if (city_id.is_not_null())
       db.delete_city(city_id);
      else
       throw joedb::Exception("city does not exist");
     }
    );
   }

   /// A procedure can return values by writing them to the message database
   void get_population(population::Writable_Database &population)
   {
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

   /// A message can have the same schema as the main database
   void get_inhabitants(tutorial::Writable_Database &message)
   {
    const auto message_city = message.get_city_table().first();

    if (message_city.is_not_null())
    {
     const auto &db = client.get_database();

     const auto city = db.find_city_by_name(message.get_name(message_city));
     if (city.is_not_null())
     {
      for (const auto person: db.get_person_table())
      {
       if (db.get_home(person) == city)
       {
        message.new_person
        (
         db.get_first_name(person),
         db.get_last_name(person),
         db.get_home(person)
        );
       }
      }
     }
    }
   }
 };
}

#endif
