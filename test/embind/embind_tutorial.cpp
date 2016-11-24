#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

#include "../../doc/source/tutorial/tutorial.h"

/////////////////////////////////////////////////////////////////////////////
void download()
/////////////////////////////////////////////////////////////////////////////
{
 printf("Starting to download...\n");
 emscripten_wget("http://localhost/tutorial.joedb", "tutorial.joedb");
 printf("Downloaded. Now opening the file...\n");
 tutorial::File_Database db("tutorial.joedb");
 if (db.is_good())
 {
  printf("File opened successfully!\n");
  for (auto person: db.get_person_table())
   printf("%s %s\n",
          db.get_first_name(person).c_str(),
          db.get_last_name(person).c_str());
 }
 else
  printf("Error opening file!\n");
}

/////////////////////////////////////////////////////////////////////////////
EMSCRIPTEN_BINDINGS(my_module)
/////////////////////////////////////////////////////////////////////////////
{
 emscripten::function("download", &download);
}
