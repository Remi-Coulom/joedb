#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

#include <fstream>

using namespace emscripten;

void download()
{
 printf("Starting to download...\n");
 emscripten_wget("http://localhost/tutorial.joedb", "tutorial.joedb");
 printf("Downloaded. Now opening the file...\n");
 std::ifstream ifs("tutorial.joedb");
 if (ifs.good())
  printf("File opened successfully!\n");
 else
  printf("Error opening file!\n");
}

EMSCRIPTEN_BINDINGS(my_module) {
    function("download", &download);
}
