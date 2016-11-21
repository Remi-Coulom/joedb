#include "tutorial_wrapper.h"

#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 tutorial_db *db = tutorial_open_file("wrapper_test.joedb", false);

 if (tutorial_is_good(db))
  printf("good!\n");
 else
 {
  printf("bad!\n");
  tutorial_delete(db);
  return 1;
 }

 tutorial_timestamp(db);
 tutorial_comment(db, "This is a comment");
 tutorial_valid_data(db);
 tutorial_checkpoint_full_commit(db);

 tutorial_set_city_name(db, tutorial_new_city(db), "Lille");
 tutorial_set_city_name(db, tutorial_new_city(db), "Paris");
 tutorial_set_city_name(db, tutorial_new_city(db), "Versailles");
 tutorial_set_city_name(db, tutorial_new_city(db), "Berlin");
 tutorial_set_city_name(db, tutorial_new_city(db), "Rome");

 tutorial_delete_city(db, 2);

 printf("List of cities:\n");
 {
  tutorial_id_of_city city;
  for (city = tutorial_get_beginning_of_city(db);
       city != tutorial_get_end_of_city(db);
       city = tutorial_get_next_city(db, city))
   printf(" %d  %s\n", (int)city, tutorial_get_city_name(db, city));
 }

 tutorial_delete(db);

 return 0;
}
