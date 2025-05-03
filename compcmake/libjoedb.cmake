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
 ${JOEDB_SRC_DIR}/joedb/journal/Readonly_Interpreted_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Interpreted_File.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Channel.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Client.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Connection.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/File_Connection.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Server_Client.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Server_Connection.cpp
 ${JOEDB_SRC_DIR}/joedb/concurrency/Server_File.cpp
)

set(JOEDB_DATABASES
 ${JOEDB_SRC_DIR}/joedb/db/encoded_file/writable.cpp
 ${JOEDB_SRC_DIR}/joedb/db/multi_server/readonly.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Encoded_File.cpp
 ${JOEDB_SRC_DIR}/joedb/journal/Readonly_Encoded_File.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/File_Parser.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Client_Parser.cpp
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
  ${JOEDB_SRC_DIR}/joedb/concurrency/Local_Channel.cpp
  ${JOEDB_SOURCES}
 )
 add_definitions(-DJOEDB_HAS_NETWORKING)
endif()

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

if (UNIX)
 add_library(joedb SHARED
  ${JOEDB_SOURCES}
  ${JOEDB_DATABASES}
 )
 set_target_properties(joedb PROPERTIES SOVERSION ${JOEDB_VERSION})
 target_uses_ipo(joedb)
else()
 add_library(joedb STATIC
  ${JOEDB_SOURCES}
  ${JOEDB_DATABASES}
 )
endif()

target_link_libraries(joedb joedb_for_joedbc ${JOEDB_EXTERNAL_LIBS})

message("-- JOEDB_SRC_DIR = ${JOEDB_SRC_DIR}")

joedbc_build_absolute(${JOEDB_SRC_DIR}/joedb/db multi_server)
joedbc_build_absolute(${JOEDB_SRC_DIR}/joedb/db encoded_file)
add_dependencies(joedb all_joedbc)
