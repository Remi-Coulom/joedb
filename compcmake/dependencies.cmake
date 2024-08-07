include_guard(GLOBAL)

# Informative messages about system configuration
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
message("-- UNIX = \"${UNIX}\"")
message("-- CMAKE_CROSSCOMPILING = \"${CMAKE_CROSSCOMPILING}\"")
message("-- CMAKE_SYSTEM_NAME = \"${CMAKE_SYSTEM_NAME}\"")
message("-- CMAKE_SIZEOF_VOID_P = \"${CMAKE_SIZEOF_VOID_P}\"")

# Necessary to get pread and pwrite in cygwin
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if(${CMAKE_SYSTEM_NAME} EQUAL CYGWIN)
 add_definitions(-D_POSIX_C_SOURCE=200809L)
endif()

# Threads
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Networking
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if(${CMAKE_SYSTEM_NAME} EQUAL CYGWIN)
 if (TRUE)
  message("-- asio does not work in cygwin")
 else()
  find_package(Boost COMPONENTS system)
  if (Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
   link_libraries(${Boost_LIBRARIES})
   add_definitions(-D_XOPEN_SOURCE=500)
   add_definitions(-DJOEDB_HAS_BOOST_NET)
   set(ASIO_FOUND TRUE)
  endif()
 endif()
else()
 set(ASIO_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../submodules/asio/asio/include)
 if (EXISTS "${ASIO_DIRECTORY}/asio/ts/net.hpp")
  message("-- Found asio in submodules")
  include_directories(${ASIO_DIRECTORY})
  add_definitions(-DJOEDB_HAS_ASIO_NET)
  set(ASIO_FOUND TRUE)
  message("== networking OK")
 else()
  message("== no networking. Try git submodule update --init --recursive")
 endif()
endif()

# libssh
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if(${CMAKE_SYSTEM_NAME} EQUAL CYGWIN AND ${CMAKE_SIZEOF_VOID_P} EQUAL "4")
 # https://cygwin.com/pipermail/cygwin/2022-January/250520.html
 message("-- libssh does not work in 32-bit cygwin")
else()
 find_package(libssh QUIET)

 if (libssh_FOUND)
  if (NOT LIBSSH_LIBRARIES)
   set(LIBSSH_LIBRARIES ssh)
  endif()
  add_definitions(-DJOEDB_HAS_SSH)
  find_path(libssh_include_path "libssh/libssh.h" NO_CACHE)
  include_directories(${libssh_include_path})
  set(JOEDB_EXTERNAL_LIBS ${JOEDB_EXTERNAL_LIBS} ${LIBSSH_LIBRARIES})
  message("== ssh was found (${LIBSSH_LIBRARIES}) (${libssh_include_path})")
 else()
  message("== ssh not found")
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   message("   suggestion: sudo apt install libssh-dev")
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   message("   suggestion: sudo port install libssh")
  elseif(WIN32)
   message("   suggestion: vcpkg install libssh:x64-windows")
  endif()
 endif()
endif()

# libcurl
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_package(CURL QUIET)
if (CURL_FOUND)
 message("== CURL found (${CURL_LIBRARIES})")
 add_definitions(-DJOEDB_HAS_CURL)
 set(JOEDB_EXTERNAL_LIBS ${JOEDB_EXTERNAL_LIBS} ${CURL_LIBRARIES})
else()
 message("== CURL not found")
 if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  message("-- Suggestion: sudo apt install libcurl4-openssl-dev")
 endif()
endif()

# Brotli
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (MSVC)
 find_package(unofficial-brotli CONFIG)
endif()

find_path(brotli_encode_include_path "brotli/encode.h" NO_CACHE)
find_library(brotli_decode_lib NAMES brotlidec)
find_library(brotli_encode_lib NAMES brotlienc)

if (brotli_encode_include_path AND brotli_decode_lib AND brotli_encode_lib)
 set(Brotli_FOUND TRUE)
 message("== Brotli found: ${brotli_encode_include_path} ${brotli_decode_lib} ${brotli_encode_lib}")
 add_definitions(-DJOEDB_HAS_BROTLI)
 include_directories(${brotli_encode_include_path})
 set(JOEDB_EXTERNAL_LIBS ${JOEDB_EXTERNAL_LIBS} ${brotli_decode_lib} ${brotli_encode_lib})
else()
 message("== Brotli not found. Try sudo apt install libbrotli-dev")
endif()

# winsock
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (WIN32)
 set(JOEDB_EXTERNAL_LIBS ${JOEDB_EXTERNAL_LIBS} wsock32 ws2_32)
endif()
