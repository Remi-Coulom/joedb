#include "Interpreter.h"
#include "Database.h"

#include <iostream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
int main()
{
 Listener listener;
 Database db(listener);
 Interpreter interpreter(db);

 interpreter.main_loop(std::cin, std::cout);

 return 0;
}
