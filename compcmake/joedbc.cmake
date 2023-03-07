set(JOEDB_DIR ${CMAKE_CURRENT_LIST_DIR})

include("${JOEDB_DIR}/dependencies.cmake")
include("${JOEDB_DIR}/libjoedb.cmake")

#############################################################################
# Custom add_executable
#############################################################################
if (CMAKE_VERSION VERSION_GREATER 3.9)
 cmake_policy(SET CMP0069 NEW)
 include(CheckIPOSupported)
 check_ipo_supported(RESULT ipo_supported)
endif()

if (ipo_supported)
 message("-- IPO supported")
else()
 message("-- IPO not supported")
endif()

function(ipo_add_executable)
 add_executable(${ARGV})
 if (ipo_supported)
  set_property(TARGET ${ARGV0} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
 endif()
endfunction(ipo_add_executable)

#############################################################################
# Joedbc executable
#############################################################################
ipo_add_executable(joedbc
 ${JOEDB_SRC_DIR}/joedb/compiler/joedbc.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/Compiler_Options_io.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/c_wrapper.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/nested_namespace.cpp
)
target_link_libraries(joedbc joedb)

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
   ${dir}/${namespace}_wrapper.cpp
   ${dir}/${namespace}_wrapper.h
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
  if (libssh_FOUND)
   target_link_libraries(${target} ${LIBSSH_LIBRARIES})
  endif()
 endif()
endfunction()

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function(joedb_add_executable)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ipo_add_executable(${ARGV})
 target_uses_joedb(${ARGV0})
endfunction()
