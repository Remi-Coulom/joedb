include_guard(GLOBAL)

include_directories(BEFORE SYSTEM ${CMAKE_CURRENT_LIST_DIR}/../src)

include("${CMAKE_CURRENT_LIST_DIR}/joedbc.cmake")

set(JOEDB_SOURCES
 ${JOEDB_SRC_DIR}/joedb/Signal.cpp
 ${JOEDB_SRC_DIR}/joedb/io/open_mode_strings.cpp
 ${JOEDB_SRC_DIR}/joedb/io/inplace_pack.cpp
 ${JOEDB_SRC_DIR}/joedb/io/merge.cpp
 ${JOEDB_SRC_DIR}/joedb/io/process_journal_pair.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Client_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Raw_Dump_Writable.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Connection_Parser.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/File_Hasher.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Stream_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Readonly_Interpreted_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Interpreted_File.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Channel.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Client.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Client_Data.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Connection.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Server_Connection.cpp
)

set(JOEDB_DATABASES
 ${JOEDB_SRC_DIR}/joedb/db/encoded_file.cpp
 ${JOEDB_SRC_DIR}/joedb/db/multi_server_readonly.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Encoded_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Readonly_Encoded_File.cpp
 ${JOEDB_SRC_DIR}/joedb/io/File_Parser.cpp
 ${JOEDB_SRC_DIR}/joedb/io/Client_Parser.cpp
)

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
  ${JOEDB_SRC_DIR}/joedb/concurrency/Server.cpp
  ${JOEDB_SRC_DIR}/joedb/concurrency/IO_Context_Wrapper.cpp
  ${JOEDB_SRC_DIR}/joedb/concurrency/Network_Channel.cpp
  ${JOEDB_SOURCES}
 )
 add_definitions(-DJOEDB_HAS_NETWORKING)
endif()

if(unofficial-brotli_FOUND)
 set(JOEDB_SOURCES ${JOEDB_SOURCES}
  ${JOEDB_SRC_DIR}/joedb/journal/Brotli_Codec.cpp
 )
 set(JOEDB_DATABASES ${JOEDB_DATABASES}
  ${JOEDB_SRC_DIR}/joedb/journal/Brotli_File.cpp
 )
endif()

add_library(joedb_no_db OBJECT ${JOEDB_SOURCES})
add_library(joedb_db OBJECT ${JOEDB_DATABASES})

if (UNIX)
 add_library(joedb SHARED
  $<TARGET_OBJECTS:joedb_no_db>
  $<TARGET_OBJECTS:joedb_db>
 )
 set_target_properties(joedb PROPERTIES SOVERSION ${JOEDB_VERSION})
 target_uses_ipo(joedb)
else()
 add_library(joedb STATIC
  $<TARGET_OBJECTS:joedb_no_db>
  $<TARGET_OBJECTS:joedb_db>
 )
endif()

target_link_libraries(joedb joedb_db joedb_no_db joedb_for_joedbc ${JOEDB_EXTERNAL_LIBS})

message("-- JOEDB_SRC_DIR = ${JOEDB_SRC_DIR}")

joedbc_build_absolute(${JOEDB_SRC_DIR}/joedb/db multi_server)
joedbc_build_absolute(${JOEDB_SRC_DIR}/joedb/db encoded_file)
add_dependencies(joedb_db all_joedbc)
