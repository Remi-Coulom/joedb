include_guard(GLOBAL)

set(JOEDB_DIR ${CMAKE_CURRENT_LIST_DIR})
set(JOEDB_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../src)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(BEFORE SYSTEM ${JOEDB_SRC_DIR})

include("${JOEDB_DIR}/dependencies.cmake")
include("${JOEDB_DIR}/ipo.cmake")
include("${JOEDB_DIR}/joedb_version.cmake")
include("${JOEDB_DIR}/defines.cmake")

if (DEFINED JOEDB_PORTABLE)
 message("-- JOEDB_PORTABLE")
 add_definitions(-DJOEDB_PORTABLE)
endif()

#############################################################################
# Part of joedb sources necessary to build joedbc
#############################################################################
add_library(joedb_for_joedbc OBJECT
 ${JOEDB_SRC_DIR}/external/wide_char_display_width.cpp
 ${JOEDB_SRC_DIR}/joedb/is_identifier.cpp
 ${JOEDB_SRC_DIR}/joedb/Multiplexer.cpp
 ${JOEDB_SRC_DIR}/joedb/Readable.cpp
 ${JOEDB_SRC_DIR}/joedb/Selective_Writable.cpp
 ${JOEDB_SRC_DIR}/joedb/Writable.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/nested_namespace.cpp
 ${JOEDB_SRC_DIR}/joedb/error/assert.cpp
 ${JOEDB_SRC_DIR}/joedb/error/Destructor_Logger.cpp
 ${JOEDB_SRC_DIR}/joedb/interpreted/Database.cpp
 ${JOEDB_SRC_DIR}/joedb/interpreted/Database_Schema.cpp
 ${JOEDB_SRC_DIR}/joedb/interpreted/Table.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Arguments.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/base64.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Blob_Reader_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Command_Interpreter.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Data_Manipulation_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/diagnostics.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/dump.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/get_time_string.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Interpreter_Dump_Writable.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/json.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/main_wrapper.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Readable_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Readable_Writable_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/SQL_Dump_Writable.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/type_io.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Writable_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/write_value.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Stream_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Async_Reader.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Buffered_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Journal_Construction_Lock.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Memory_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Readonly_Journal.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Writable_Journal.cpp
)
target_link_libraries(joedb_for_joedbc ${JOEDB_EXTERNAL_LIBS})

#############################################################################
# Joedbc executable
#############################################################################
add_library(joedbc_objects OBJECT
 ${JOEDB_SRC_DIR}/joedb/compiler/joedbc.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/Compiler_Options_io.cpp

 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Generator.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Database_h.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Database_Writable_h.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Database_Writable_cpp.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Readonly_Database_h.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Types_h.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Readable_h.cpp

 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Writable_Database_h.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Writable_Database_cpp.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/File_Database_h.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Memory_Database_h.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Interpreted_File_Database_h.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Readonly_Interpreted_File_Database_h.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Multiplexer_h.cpp

 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Client_h.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/File_Client_h.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/Readonly_Client_h.cpp

 ${JOEDB_SRC_DIR}/joedb/compiler/generator/ids_h.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/generator/introspection_h.cpp
)
target_link_libraries(joedbc_objects ${JOEDB_EXTERNAL_LIBS})

ipo_add_executable(joedbc
 $<TARGET_OBJECTS:joedbc_objects>
 $<TARGET_OBJECTS:joedb_for_joedbc>
)

target_link_libraries(joedbc ${JOEDB_EXTERNAL_LIBS})

#############################################################################
# Functions to create dependencies for joedbc
#############################################################################

add_custom_target(all_joedbc)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function(joedbc_build_absolute dir namespace)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 add_custom_command(
  OUTPUT
   ${dir}/${namespace}/readonly.cpp
   ${dir}/${namespace}/readonly.h
   ${dir}/${namespace}/writable.cpp
   ${dir}/${namespace}/writable.h
  COMMAND joedbc ${namespace}.joedbi ${namespace}.joedbc
  DEPENDS
   joedbc
   ${dir}/${namespace}.joedbi
   ${dir}/${namespace}.joedbc
  WORKING_DIRECTORY ${dir}
 )
 add_custom_target(compile_${namespace}_with_joedbc
  DEPENDS
   ${dir}/${namespace}/readonly.cpp
   ${dir}/${namespace}/readonly.h
   ${dir}/${namespace}/writable.cpp
   ${dir}/${namespace}/writable.h
 )
 add_dependencies(all_joedbc compile_${namespace}_with_joedbc)
endfunction()

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function(joedbc_build dir namespace)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 joedbc_build_absolute(${CMAKE_CURRENT_SOURCE_DIR}/${dir} ${namespace} ${ARGN})
endfunction()

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function(target_uses_joedb target)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 add_dependencies(${target} all_joedbc)
 target_include_directories(${target} PUBLIC ${JOEDB_DIR}/../src)
 get_target_property(target_type ${target} TYPE)

 if (NOT "${target_type}" STREQUAL "OBJECT_LIBRARY")
  target_link_libraries(${target} joedb ${JOEDB_EXTERNAL_LIBS})
 endif()
endfunction()

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function(joedb_add_executable)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ipo_add_executable(${ARGV})
 target_uses_joedb(${ARGV0})
endfunction()

include("${JOEDB_DIR}/libjoedb.cmake")
