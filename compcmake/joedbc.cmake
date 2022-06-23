if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
 set(JOEDB_BUILD_PATH "out/build/x64-Release")
else()
 set(JOEDB_BUILD_PATH "gcc_release")
endif()

set(JOEDB_DIR ${CMAKE_CURRENT_LIST_DIR})

find_program(JOEDBC joedbc
 HINTS
  ${JOEDB_DIR}/${JOEDB_BUILD_PATH}
)

find_library(JOEDB_LIB joedb
 HINTS
  ${JOEDB_DIR}/${JOEDB_BUILD_PATH}
)

message("== JOEDBC = ${JOEDBC}")
message("== JOEDB_LIB = ${JOEDB_LIB}")

include("${JOEDB_DIR}/libssh.cmake")

add_custom_target(all_joedbc)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function(joedbc_build dir namespace)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 if(JOEDBC)
  add_custom_command(
   OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${namespace}_interpreted.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${namespace}_interpreted.h
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${namespace}_readonly.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${namespace}_readonly.h
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${namespace}.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${namespace}.h
   COMMAND ${JOEDBC} ${namespace}.joedbi ${namespace}.joedbc
   DEPENDS
    ${JOEDBC}
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${namespace}.joedbi
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${namespace}.joedbc
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir}
  )
  add_custom_target(compile_${namespace}_with_joedbc
   DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${namespace}.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${namespace}.h
  )
 else()
  add_custom_target(compile_${namespace}_with_joedbc)
  message("== No joedbc, not adding joedbc rule for ${namespace}")
 endif()
 add_dependencies(all_joedbc compile_${namespace}_with_joedbc)
endfunction(joedbc_build)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function(target_uses_joedb target)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 add_dependencies(${target} all_joedbc)
 target_include_directories(${target} PUBLIC ${JOEDB_DIR}/../src)
 target_link_libraries(${target} ${JOEDB_LIB})
 if (libssh_FOUND)
  target_link_libraries(${target} ${LIBSSH_LIBRARIES})
 endif()
endfunction(target_uses_joedb)
