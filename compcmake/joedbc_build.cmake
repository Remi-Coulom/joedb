include_guard(GLOBAL)

if(NOT DEFINED JOEDBC_EXECUTABLE)
 if(TARGET joedb::joedbc)
  set(JOEDBC_EXECUTABLE joedb::joedbc)
 elseif(TARGET joedbc)
  set(JOEDBC_EXECUTABLE joedbc)
 else()
  message(FATAL_ERROR
   "joedbc_build.cmake requires joedb::joedbc or joedbc to be available")
 endif()
endif()

if(NOT TARGET all_joedbc)
 add_custom_target(all_joedbc)
endif()

function(joedbc_build_absolute dir namespace)
 file(GLOB joedbis CONFIGURE_DEPENDS
  "${dir}/${namespace}.joedbi"
  "${dir}/${namespace}.rpc/*.joedbi"
 )

 file(GLOB joedbcs CONFIGURE_DEPENDS
  "${dir}/${namespace}.joedbc"
  "${dir}/${namespace}.rpc/*.joedbc"
 )

 file(GLOB service_headers CONFIGURE_DEPENDS
  "${dir}/${namespace}.rpc/Service.h"
 )

 if(NOT joedbis OR NOT joedbcs)
  message(FATAL_ERROR
   "Cannot find joedbi or joedbc for joedbc_build ${dir} ${namespace}")
 endif()

 set(slash_list ${joedbis})
 set(slash_service ${service_headers})
 list(TRANSFORM slash_list REPLACE
  "${namespace}\\.rpc" "${namespace}/rpc")
 list(TRANSFORM slash_service REPLACE
  "${namespace}\\.rpc" "${namespace}/rpc")

 set(readonly_cpp ${slash_list})
 set(writable_cpp ${slash_list})
 list(TRANSFORM readonly_cpp REPLACE "\\.joedbi$" "/readonly.cpp")
 list(TRANSFORM writable_cpp REPLACE "\\.joedbi$" "/writable.cpp")

 set(generated_outputs
  ${readonly_cpp}
  ${writable_cpp}
  "${dir}/${namespace}/Database.h"
  ${slash_service}
 )

 add_custom_command(
  OUTPUT ${generated_outputs}
  COMMAND ${JOEDBC_EXECUTABLE} ${namespace}
  DEPENDS ${JOEDBC_EXECUTABLE} ${joedbis} ${joedbcs} ${service_headers}
  WORKING_DIRECTORY "${dir}"
  VERBATIM
 )

 set(target_name "compile_${namespace}_with_joedbc")
 if(TARGET ${target_name})
  message("== warning: target ${target_name} already exists")
 else()
  add_custom_target(${target_name} DEPENDS ${generated_outputs})
  add_dependencies(all_joedbc ${target_name})
 endif()
endfunction()

function(joedbc_build dir namespace)
 joedbc_build_absolute("${CMAKE_CURRENT_SOURCE_DIR}/${dir}" "${namespace}")
endfunction()
