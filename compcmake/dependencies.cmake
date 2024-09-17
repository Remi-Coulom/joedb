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
find_package(asio CONFIG)
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (asio_FOUND)
 message("-- asio found")
 add_definitions(-DJOEDB_HAS_ASIO_NET)
 list(APPEND JOEDB_EXTERNAL_LIBS asio::asio)
 if (WIN32)
  list(APPEND JOEDB_EXTERNAL_LIBS wsock32 ws2_32)
 endif()
else()
 message("## asio not found")
endif()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_package(libssh CONFIG)
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (libssh_FOUND)
 message("-- ssh found")
 add_definitions(-DJOEDB_HAS_SSH)
 list(APPEND JOEDB_EXTERNAL_LIBS ssh)
else()
 message("## ssh not found")
endif()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_package(CURL QUIET)
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
 message("-- Brotli found")
 add_definitions(-DJOEDB_HAS_BROTLI)
 list(APPEND JOEDB_EXTERNAL_LIBS unofficial::brotli::brotlidec)
 list(APPEND JOEDB_EXTERNAL_LIBS unofficial::brotli::brotlienc)
else()
 message("## Brotli not found")
endif()
