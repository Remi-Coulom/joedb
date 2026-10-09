include_guard(GLOBAL)

include(GNUInstallDirs)

include_directories(BEFORE SYSTEM ${CMAKE_CURRENT_LIST_DIR}/../src)

include("${CMAKE_CURRENT_LIST_DIR}/joedbc.cmake")

set(JOEDB_SOURCES
 ${JOEDB_SRC_DIR}/joedb/Signal.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/open_mode_strings.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/inplace_pack.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/interruptible_sleep.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/process_journal_pair.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Client_Command_Processor.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Raw_Dump_Writable.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Connection_Parser.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Parsed_Logger.cpp
 ${JOEDB_SRC_DIR}/joedb/ui/Progress_Bar.cpp
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
 ${JOEDB_SRC_DIR}/joedb/ui/Parsed_Client.cpp
)

if(JOEDB_HAS_BROTLI)
 set(JOEDB_SOURCES ${JOEDB_SOURCES}
  ${JOEDB_SRC_DIR}/joedb/journal/Brotli_Decoder.cpp
  ${JOEDB_SRC_DIR}/joedb/journal/Brotli_Codec.cpp
 )
 set(JOEDB_DATABASES ${JOEDB_DATABASES}
  ${JOEDB_SRC_DIR}/joedb/journal/Readonly_Brotli_File.cpp
  ${JOEDB_SRC_DIR}/joedb/journal/Brotli_File.cpp
 )
endif()

if(JOEDB_HAS_SSH)
 set(JOEDB_SOURCES ${JOEDB_SOURCES}
  ${JOEDB_SRC_DIR}/joedb/ssh/Forward_Channel.cpp
  ${JOEDB_SRC_DIR}/joedb/ssh/SFTP.cpp
 )
endif()

if(JOEDB_HAS_CURL)
 set(JOEDB_SOURCES ${JOEDB_SOURCES}
  ${JOEDB_SRC_DIR}/joedb/journal/CURL_File.cpp
 )
endif()

if(JOEDB_HAS_ASIO)
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

add_library(joedb
 $<TARGET_OBJECTS:joedb_sources>
 $<TARGET_OBJECTS:joedb_for_joedbc>
 $<TARGET_OBJECTS:joedb_databases>
)

if(BUILD_SHARED_LIBS)
 set_target_properties(joedb PROPERTIES SOVERSION ${JOEDB_VERSION})
 target_uses_ipo(joedb)
endif()

add_library(joedb::joedb ALIAS joedb)

target_include_directories(joedb PUBLIC
 $<BUILD_INTERFACE:${JOEDB_SRC_DIR}>
 $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(joedb PUBLIC cxx_std_17)

target_link_libraries(joedb PUBLIC ${JOEDB_EXTERNAL_LIBS})

if(JOEDB_HAS_BOOST)
 target_compile_definitions(joedb PUBLIC JOEDB_HAS_BOOST)
endif()

if(JOEDB_HAS_ASIO)
 target_compile_definitions(joedb PUBLIC JOEDB_HAS_ASIO)
endif()

if(JOEDB_HAS_SSH)
 target_compile_definitions(joedb PUBLIC JOEDB_HAS_SSH)
endif()

if(JOEDB_HAS_CURL)
 target_compile_definitions(joedb PUBLIC JOEDB_HAS_CURL)
endif()

if(JOEDB_HAS_BROTLI)
 target_compile_definitions(joedb PUBLIC JOEDB_HAS_BROTLI)
endif()

if(JOEDB_HAS_WEBSOCKETS)
 target_compile_definitions(joedb PUBLIC JOEDB_HAS_WEBSOCKETS)
endif()

if(DEFINED JOEDB_PORTABLE)
 target_compile_definitions(joedb PUBLIC JOEDB_PORTABLE)
endif()

message("-- JOEDB_SRC_DIR = ${JOEDB_SRC_DIR}")
