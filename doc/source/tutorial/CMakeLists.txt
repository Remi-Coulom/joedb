cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0069 NEW)

project(tutorial)

set(CMAKE_CXX_STANDARD 17)

include("../../../compcmake/joedbc.cmake")

#
# Including "joedbc.cmake" defines four functions
#
#  * joedbc_build(<dir> <name>): add rules to compile a database with joedbc
#     <dir> is relative to ${CMAKE_CURRENT_SOURCE_DIR}
#     Assumes that <dir>/<name>.joedbi and <dir>/<name>.joedbc contain compiler
#     instructions to generate <dir>/<name>.cpp
#     This function may be invoked multiple times, once for each database
#     contained in the code.
#
#  * joedbc_build_absolute(<dir> <name>): same as above, but <dir> is absolute
#
#  * target_uses_joedb(target): indicate that a target uses joedb. Two effects:
#     1: it adds a dependency, so that joedbc is invoked whenever necessary
#     2: it links the executable to the joedb library
#
#  * joedb_add_executable(target source...): add an executable that uses joedb
#
#  joedbc.cmake will compile joedbc and the joedb library for your project,
#  with the same compiler and compilation options as the rest of your code.
#

joedbc_build("." tutorial)
joedbc_build("." settings)

joedb_add_executable(tutorial tutorial_main.cpp tutorial.cpp)

joedb_add_executable(local_concurrency local_concurrency.cpp tutorial.cpp)

joedb_add_executable(file_tutorial file_tutorial.cpp tutorial.cpp)

joedb_add_executable(concurrency_tutorial concurrency_tutorial.cpp tutorial.cpp)

joedb_add_executable(index_tutorial index_tutorial.cpp tutorial.cpp)

joedb_add_executable(settings settings_main.cpp settings.cpp)
