include_guard(GLOBAL)

include_directories(BEFORE SYSTEM ${CMAKE_CURRENT_LIST_DIR}/../src)

include("${CMAKE_CURRENT_LIST_DIR}/ipo.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/compiler_flags.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/dependencies.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/joedb_version.cmake")

set(JOEDB_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../src)

if (DEFINED JOEDB_PORTABLE)
 message("-- JOEDB_PORTABLE")
 add_definitions(-DJOEDB_PORTABLE)
endif()

set(JOEDB_SOURCES
 ${JOEDB_SRC_DIR}/external/wide_char_display_width.cpp
 ${JOEDB_SRC_DIR}/joedb/Blob.cpp
 ${JOEDB_SRC_DIR}/joedb/Destructor_Logger.cpp
 ${JOEDB_SRC_DIR}/joedb/is_identifier.cpp
 ${JOEDB_SRC_DIR}/joedb/Multiplexer.cpp
 ${JOEDB_SRC_DIR}/joedb/Posthumous_Catcher.cpp
 ${JOEDB_SRC_DIR}/joedb/Posthumous_Thrower.cpp
 ${JOEDB_SRC_DIR}/joedb/Readable.cpp
 ${JOEDB_SRC_DIR}/joedb/Selective_Writable.cpp
 ${JOEDB_SRC_DIR}/joedb/Signal.cpp
 ${JOEDB_SRC_DIR}/joedb/Writable.cpp
 ${JOEDB_SRC_DIR}/joedb/interpreter/Database.cpp
 ${JOEDB_SRC_DIR}/joedb/interpreter/Database_Schema.cpp
 ${JOEDB_SRC_DIR}/joedb/interpreter/Table.cpp
 ${JOEDB_SRC_DIR}/joedb/io/base64.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/io/get_time_string.cpp
 ${JOEDB_SRC_DIR}/joedb/io/open_mode_strings.cpp
 ${JOEDB_SRC_DIR}/joedb/io/dump.cpp
 ${JOEDB_SRC_DIR}/joedb/io/inplace_pack.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Command_Interpreter.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Client_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Client_Parser.cpp
 ${JOEDB_SRC_DIR}/joedb/io/File_Parser.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Readable_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Writable_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Readable_Writable_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Interpreter_Dump_Writable.cpp
 ${JOEDB_SRC_DIR}/joedb/io/json.cpp
 ${JOEDB_SRC_DIR}/joedb/io/main_exception_catcher.cpp
 ${JOEDB_SRC_DIR}/joedb/io/merge.cpp
 ${JOEDB_SRC_DIR}/joedb/io/process_journal_pair.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Raw_Dump_Writable.cpp
 ${JOEDB_SRC_DIR}/joedb/io/SQL_Dump_Writable.cpp
 ${JOEDB_SRC_DIR}/joedb/io/type_io.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Connection_Parser.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Abstract_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Generic_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Journal_Construction_Lock.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Memory_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Writable_Journal.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Readonly_Journal.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/diagnostics.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Stream_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/SHA_256.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Interpreted_File.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Channel.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Client.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Client_Data.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Connection.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Server_Connection.cpp
 ${JOEDB_SRC_DIR}/joedb/compiler/nested_namespace.cpp
)

if (ASIO_FOUND)
 set(JOEDB_SOURCES ${JOEDB_SOURCES}
  ${JOEDB_SRC_DIR}/joedb/concurrency/IO_Context_Wrapper.cpp
  ${JOEDB_SRC_DIR}/joedb/concurrency/Server.cpp
  ${JOEDB_SRC_DIR}/joedb/concurrency/Network_Channel.cpp
 )
 add_definitions(-DJOEDB_HAS_NETWORKING)
endif()

if(libssh_FOUND)
 set(JOEDB_SOURCES ${JOEDB_SOURCES}
  ${JOEDB_SRC_DIR}/joedb/ssh/Forward_Channel.cpp
  ${JOEDB_SRC_DIR}/joedb/ssh/SFTP.cpp
 )
endif()

if(CURL_FOUND)
 set(JOEDB_SOURCES ${JOEDB_SOURCES}
  ${JOEDB_SRC_DIR}/joedb/journal/CURL_File.cpp
 )
endif()

if (UNIX)
 add_library(joedb SHARED ${JOEDB_SOURCES})
 set_target_properties(joedb PROPERTIES SOVERSION ${JOEDB_VERSION})
 target_uses_ipo(joedb)
else()
 # In Windows, symbols of a DLL must be explicitly exported
 add_library(joedb STATIC ${JOEDB_SOURCES})
endif()

target_link_libraries(joedb Threads::Threads)

if(libssh_FOUND)
 target_link_libraries(joedb ${LIBSSH_LIBRARIES})
endif()

if(CURL_FOUND)
 target_link_libraries(joedb ${CURL_LIBRARIES})
endif()

include("${CMAKE_CURRENT_LIST_DIR}/defines.cmake")
