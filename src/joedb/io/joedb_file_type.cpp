#include "joedb/journal/File.h"

#include <iostream>

#define STRINGIFY_1(x) #x
#define STRINGIFY_2(x) STRINGIFY_1(x)

int main()
{
 std::cout << "File is " << STRINGIFY_2(JOEDB_FILE) << '\n';
 return 0;
}
