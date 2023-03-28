cmake_minimum_required(VERSION 3.1)

project(tutorial)

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
#  joedbc.cmake searches for joedbc and libjoedb in standard locations, giving
#  a priority to the local release build. If necessary, you can edit
#  joedbc.cmake so that it finds them elsewhere.
#

joedbc_build("." tutorial)

joedb_add_executable(tutorial tutorial_main.cpp tutorial.cpp)

joedb_add_executable(local_concurrency local_concurrency.cpp tutorial.cpp)

joedb_add_executable(file_tutorial file_tutorial.cpp tutorial.cpp)
