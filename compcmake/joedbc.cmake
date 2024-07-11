include_guard(GLOBAL)

set(JOEDB_DIR ${CMAKE_CURRENT_LIST_DIR})

include("${JOEDB_DIR}/dependencies.cmake")
include("${JOEDB_DIR}/libjoedb.cmake")
include("${JOEDB_DIR}/ipo.cmake")

#############################################################################
# Joedbc executable
#############################################################################
ipo_add_executable(joedbc
 ${JOEDB_SRC_DIR}/joedb/compiler/joedbc.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/Compiler_Options_io.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/c_wrapper.cpp
)
target_link_libraries(joedbc joedb_bootstrap)

#############################################################################
# Functions to create dependencies for joedbc
#############################################################################

add_custom_target(all_joedbc)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function(joedbc_build_absolute dir namespace)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 add_custom_command(
  OUTPUT
   ${dir}/${namespace}_interpreted.cpp
   ${dir}/${namespace}_interpreted.h
   ${dir}/${namespace}_readonly.cpp
   ${dir}/${namespace}_readonly.h
   ${dir}/${namespace}.cpp
   ${dir}/${namespace}.h
  COMMAND joedbc ${namespace}.joedbi ${namespace}.joedbc
  DEPENDS
   joedbc
   ${dir}/${namespace}.joedbi
   ${dir}/${namespace}.joedbc
  WORKING_DIRECTORY ${dir}
 )
 add_custom_target(compile_${namespace}_with_joedbc
  DEPENDS
   ${dir}/${namespace}.cpp
   ${dir}/${namespace}.h
 )
 add_dependencies(all_joedbc compile_${namespace}_with_joedbc)
endfunction()

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function(joedbc_build dir namespace)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 joedbc_build_absolute(${CMAKE_CURRENT_SOURCE_DIR}/${dir} ${namespace})
endfunction()

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function(target_uses_joedb target)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 add_dependencies(${target} all_joedbc)
 target_include_directories(${target} PUBLIC ${JOEDB_DIR}/../src)
 get_target_property(target_type ${target} TYPE)

 if (NOT "${target_type}" STREQUAL "OBJECT_LIBRARY")
  target_link_libraries(${target} joedb)
 endif()
endfunction()

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function(joedb_add_executable)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ipo_add_executable(${ARGV})
 target_uses_joedb(${ARGV0})
endfunction()

joedbc_build(../src/joedb/db multi_server)
joedbc_build(../src/joedb/db encoded_file)
add_dependencies(joedb all_joedbc)
