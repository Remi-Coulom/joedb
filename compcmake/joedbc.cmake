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
function(joedbc_build_absolute dir namespace)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 if(JOEDBC)
  add_custom_command(
   OUTPUT
    ${dir}/${namespace}_interpreted.cpp
    ${dir}/${namespace}_interpreted.h
    ${dir}/${namespace}_readonly.cpp
    ${dir}/${namespace}_readonly.h
    ${dir}/${namespace}.cpp
    ${dir}/${namespace}.h
   COMMAND ${JOEDBC} ${namespace}.joedbi ${namespace}.joedbc
   DEPENDS
    ${JOEDBC}
    ${dir}/${namespace}.joedbi
    ${dir}/${namespace}.joedbc
   WORKING_DIRECTORY ${dir}
  )
  add_custom_target(compile_${namespace}_with_joedbc
   DEPENDS
    ${dir}/${namespace}.cpp
    ${dir}/${namespace}.h
  )
 else()
  add_custom_target(compile_${namespace}_with_joedbc)
  message("== No joedbc, not adding joedbc rule for ${namespace}")
 endif()
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
 target_link_libraries(${target} ${JOEDB_LIB})
 if (libssh_FOUND)
  target_link_libraries(${target} ${LIBSSH_LIBRARIES})
 endif()
endfunction()
