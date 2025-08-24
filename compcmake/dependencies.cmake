include_guard(GLOBAL)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Informative messages about system configuration
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
message("-- UNIX = \"${UNIX}\"")
message("-- CMAKE_CROSSCOMPILING = \"${CMAKE_CROSSCOMPILING}\"")
message("-- CMAKE_SYSTEM_NAME = \"${CMAKE_SYSTEM_NAME}\"")
message("-- CMAKE_SIZEOF_VOID_P = \"${CMAKE_SIZEOF_VOID_P}\"")

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Threads
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_package(Boost CONFIG COMPONENTS system)
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (Boost_FOUND)
 message("-- boost found")
 add_definitions(-DJOEDB_HAS_BOOST)

 find_path(
  CERTIFY_INCLUDES boost/certify/https_verification.hpp
  HINTS ${CMAKE_CURRENT_LIST_DIR}/../../certify/include
 )
 if (CERTIFY_INCLUDES)
  message("-- certify found: ${CERTIFY_INCLUDES}")
  add_definitions(-DJOEDB_HAS_WEBSOCKETS)
  include_directories(${CERTIFY_INCLUDES})
 else()
  message("## boost::certify not found, disabling websockets")
  message("## Fix: next to joedb, git clone https://github.com/djarek/certify.git")
 endif()
 include_directories(${Boost_INCLUDE_DIRS} ../../certify/include)

 find_path(ASIO_INCLUDES boost/asio.hpp)
 if (ASIO_INCLUDES)
  message("-- asio found: ${ASIO_INCLUDES}")
  set(asio_FOUND TRUE)
  include_directories(${ASIO_INCLUDES})
  add_definitions(-DJOEDB_HAS_ASIO)
  if (WIN32)
   list(APPEND JOEDB_EXTERNAL_LIBS wsock32 ws2_32)
  endif()
 else()
  message("## asio not found")
 endif()

else()
 message("## boost not found. Fix: sudo apt install libboost-system-dev")
endif()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_package(libssh CONFIG)
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (NOT libssh_FOUND)
 find_package(libssh QUIET)
endif()

if (libssh_FOUND)
 message("-- ssh found")
 add_definitions(-DJOEDB_HAS_SSH)
 list(APPEND JOEDB_EXTERNAL_LIBS ssh)
else()
 message("## ssh not found")
endif()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (NOT JOEDB_NO_CURL)
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 find_package(CURL QUIET)
endif()

if (CURL_FOUND)
 message("-- CURL found")
 add_definitions(-DJOEDB_HAS_CURL)
 list(APPEND JOEDB_EXTERNAL_LIBS CURL::libcurl)
else()
 message("## CURL not found")
endif()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_package(unofficial-brotli CONFIG)
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (unofficial-brotli_FOUND)
 list(APPEND JOEDB_EXTERNAL_LIBS unofficial::brotli::brotlidec)
 list(APPEND JOEDB_EXTERNAL_LIBS unofficial::brotli::brotlienc)
else()
 find_path(brotli_encode_include_path "brotli/encode.h")
 find_library(brotli_decode_lib NAMES brotlidec)
 find_library(brotli_encode_lib NAMES brotlienc)
 if (brotli_encode_include_path AND brotli_decode_lib AND brotli_encode_lib)
  set(unofficial-brotli_FOUND TRUE)
  include_directories(${brotli_encode_include_path})
  list(APPEND JOEDB_EXTERNAL_LIBS ${brotli_decode_lib} ${brotli_encode_lib})
 endif()
endif()

if (unofficial-brotli_FOUND)
 message("-- Brotli found")
 add_definitions(-DJOEDB_HAS_BROTLI)
else()
 message("## Brotli not found")
endif()
