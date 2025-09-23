include_guard(GLOBAL)

include_directories(BEFORE SYSTEM ${CMAKE_CURRENT_LIST_DIR}/../src)

include("${CMAKE_CURRENT_LIST_DIR}/joedbc.cmake")

set(JOEDB_SOURCES
 ${JOEDB_SRC_DIR}/joedb/Signal.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/open_mode_strings.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/inplace_pack.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/merge.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/process_journal_pair.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Progress_Bar.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Client_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Raw_Dump_Writable.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Connection_Parser.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/File_Hasher.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Interpreted_File.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Channel.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Client.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Connection.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/File_Connection.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Robust_Connection.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Server_Client.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Server_Connection.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Server_File.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Websocket_Channel.cpp
 ${JOEDB_SRC_DIR}/joedb/rpc/get_hash.cpp
 ${JOEDB_SRC_DIR}/joedb/error/System_Logger.cpp
)

set(JOEDB_DATABASES
 ${JOEDB_SRC_DIR}/joedb/db/encoded_file/writable.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Encoded_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Readonly_Encoded_File.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/File_Parser.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Client_Parser.cpp
)

if(unofficial-brotli_FOUND)
 set(JOEDB_SOURCES ${JOEDB_SOURCES}
  ${JOEDB_SRC_DIR}/joedb/journal/Brotli_Decoder.cpp
  ${JOEDB_SRC_DIR}/joedb/journal/Brotli_Codec.cpp
 )
 set(JOEDB_DATABASES ${JOEDB_DATABASES}
  ${JOEDB_SRC_DIR}/joedb/journal/Readonly_Brotli_File.cpp
  ${JOEDB_SRC_DIR}/joedb/journal/Brotli_File.cpp
 )
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

if (asio_FOUND)
 set(JOEDB_SOURCES
  ${JOEDB_SRC_DIR}/joedb/concurrency/Local_Channel.cpp
  ${JOEDB_SOURCES}
 )
 if (CMAKE_CXX_STANDARD GREATER_EQUAL 20)
  set(JOEDB_SOURCES
   ${JOEDB_SRC_DIR}/joedb/asio/Server.cpp
   ${JOEDB_SRC_DIR}/joedb/concurrency/Server.cpp
   ${JOEDB_SOURCES}
  )
 endif()
endif()

add_library(joedb_databases OBJECT ${JOEDB_DATABASES})
joedbc_build_absolute(${JOEDB_SRC_DIR}/joedb/db encoded_file)
add_dependencies(joedb_databases compile_encoded_file_with_joedbc)
target_link_libraries(joedb_databases ${JOEDB_EXTERNAL_LIBS})

add_library(joedb_sources OBJECT ${JOEDB_SOURCES})
target_link_libraries(joedb_sources ${JOEDB_EXTERNAL_LIBS})

if (UNIX)
 add_library(joedb SHARED
  $<TARGET_OBJECTS:joedb_sources>
  $<TARGET_OBJECTS:joedb_for_joedbc>
  $<TARGET_OBJECTS:joedb_databases>
 )
 set_target_properties(joedb PROPERTIES SOVERSION ${JOEDB_VERSION})
 target_uses_ipo(joedb)
else()
 add_library(joedb STATIC
  $<TARGET_OBJECTS:joedb_sources>
  $<TARGET_OBJECTS:joedb_databases>
  $<TARGET_OBJECTS:joedb_for_joedbc>
 )
endif()

target_link_libraries(joedb ${JOEDB_EXTERNAL_LIBS})

message("-- JOEDB_SRC_DIR = ${JOEDB_SRC_DIR}")
