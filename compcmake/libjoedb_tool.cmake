include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/joedbc.cmake")

joedbc_build(../src/joedb/db multi_server)
joedbc_build(../src/joedb/db encoded_file)

set(JOEDB_TOOL_SOURCES
 ${JOEDB_SRC_DIR}/joedb/io/Client_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Client_Parser.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Connection_Parser.cpp
 ${JOEDB_SRC_DIR}/joedb/io/File_Parser.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Brotli_Codec.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Brotli_File.cpp
 ${JOEDB_SRC_DIR}/joedb/db/encoded_file.cpp
 ${JOEDB_SRC_DIR}/joedb/db/multi_server_readonly.cpp
 ${JOEDB_SRC_DIR}/joedb/db/multi_server_interpreted.cpp
)

if (UNIX)
 add_library(joedb_tool SHARED ${JOEDB_TOOL_SOURCES})
 set_target_properties(joedb_tool PROPERTIES SOVERSION ${JOEDB_VERSION})
 target_uses_ipo(joedb_tool)
else()
 add_library(joedb_tool STATIC ${JOEDB_TOOL_SOURCES})
endif()

target_uses_joedb(joedb_tool)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function(tool_add_executable)
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 joedb_add_executable(${ARGV})
 target_link_libraries(${ARGV0} joedb_tool)
endfunction()

