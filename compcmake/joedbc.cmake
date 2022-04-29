find_program(JOEDBC joedbc
 HINTS
  ~/repos/joedb/compcmake/gcc_release
  ~/repos/joedb/compcmake/out/build/x64-Release
  ../../../../../usr/local/bin
  ../../../../../../usr/local/bin
  ../../../../../../../usr/local/bin
)

find_library(JOEDB_LIB joedb
 HINTS
  ~/repos/joedb/compcmake/gcc_release
  ~/repos/joedb/compcmake/out/build/x64-Release
  ../../../../../usr/local/lib
  ../../../../../../usr/local/lib
  ../../../../../../../usr/local/lib
)

message("== JOEDBC = ${JOEDBC}")
message("== JOEDB_LIB = ${JOEDB_LIB}")

add_custom_target(all_joedbc)

function(joedbc_build dir namespace)
 if(JOEDBC)
  add_custom_command(
   OUTPUT
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
function(target_uses_joedb)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 add_dependencies(${ARGV0} all_joedbc)
 target_link_libraries(${ARGV0} ${JOEDB_LIB})
endfunction(target_uses_joedb)
