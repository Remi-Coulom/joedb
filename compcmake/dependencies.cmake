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
find_package(Boost COMPONENTS system)
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (Boost_FOUND)
 add_definitions(-DJOEDB_HAS_BEAST)
 include_directories(${Boost_INCLUDE_DIRS} ../../certify/include)
 message("-- boost found")
else()
 message("## boost not found")
endif()

# TODO: use asio from boost if boost is found
# TODO: handle dependency to boost::certify properly

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_package(asio CONFIG)
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (asio_FOUND)
 list(APPEND JOEDB_EXTERNAL_LIBS asio::asio)
else()
 find_path(ASIO_INCLUDES asio.hpp)
 if (ASIO_INCLUDES)
  set(asio_FOUND TRUE)
  include_directories(${ASIO_INCLUDES})
 endif()
endif()

if (asio_FOUND)
 message("-- asio found")
 add_definitions(-DJOEDB_HAS_ASIO)
 if (WIN32)
  list(APPEND JOEDB_EXTERNAL_LIBS wsock32 ws2_32)
 endif()
else()
 message("## asio not found")
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
