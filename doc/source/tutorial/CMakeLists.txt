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
#     instructions to generate source stored in the <dir>/<name> directory
#     This function may be invoked multiple times, once for each database
#     contained in the code.
#
#  * joedbc_build_absolute(<dir> <name>): same as above, but <dir> is absolute
#
#  * target_uses_joedb(target): indicate that a target uses joedb. Two effects:
#     1: it adds a dependency, so that joedbc is invoked whenever necessary
#     2: it links the executable to the joedb library
#
#  * joedb_add_executable(target source...): shortcut to add an executable
#     that uses joedb in a single command
#
#  joedbc.cmake will compile joedbc and the joedb library for your project,
#  with the same compiler and compilation options as the rest of your code.
#

joedbc_build("src" tutorial)
joedbc_build("src" settings)

joedb_add_executable(tutorial
 src/tutorial_main.cpp
 src/tutorial/writable.cpp
)

joedb_add_executable(tutorial_interpreter
 src/tutorial_interpreter.cpp
 src/tutorial/writable.cpp
)

joedb_add_executable(local_concurrency
 src/local_concurrency.cpp
 src/tutorial/writable.cpp
)

joedb_add_executable(file_tutorial
 src/file_tutorial.cpp
 src/tutorial/writable.cpp
)

joedb_add_executable(concurrency_tutorial
 src/concurrency_tutorial.cpp
 src/tutorial/writable.cpp
)

joedb_add_executable(index_tutorial
 src/index_tutorial.cpp
 src/tutorial/writable.cpp
)

joedb_add_executable(settings
 src/settings_main.cpp
 src/settings/writable.cpp
)

joedb_add_executable(write_server_blob
 src/write_server_blob.cpp
)

joedb_add_executable(client_lock
 src/client_lock.cpp
 src/tutorial/writable.cpp
)

joedb_add_executable(step_by_step_replay
 src/step_by_step_replay.cpp
 src/tutorial/readonly.cpp
)
