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

 tutorial_delete(db);

 return 0;
}
