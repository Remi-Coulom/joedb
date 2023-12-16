# Threads
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Networking
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
message("-- CMAKE_CROSSCOMPILING = ${CMAKE_CROSSCOMPILING}")
if(NOT ${CMAKE_CROSSCOMPILING})
 set(ASIO_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../submodules/asio/asio/include)
 if (TRUE)
  if (EXISTS "${ASIO_DIRECTORY}/asio/ts/net.hpp")
   message("-- Found asio in submodules")
   include_directories(${ASIO_DIRECTORY})
   add_definitions(-DJOEDB_HAS_ASIO_NET)
   set(HAS_NETWORKING TRUE)

   if(${CMAKE_SYSTEM_NAME} EQUAL CYGWIN)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-D__USE_W32_SOCKETS)
   endif()

   message("== networking OK")

  else()
   message("== no networking. Try git submodule update --init --recursive")
  endif()
 endif()
endif()

# libssh
# not compatible with asio in cygwin (because asio insists on using winsock)
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if(NOT ${CMAKE_CROSSCOMPILING})
 if(NOT ${CMAKE_SYSTEM_NAME} EQUAL CYGWIN)
  if(NOT ${CMAKE_SYSTEM_NAME} EQUAL Darwin)
   find_package(libssh QUIET)
  endif()

  if (libssh_FOUND)
   if (NOT LIBSSH_LIBRARIES)
    set(LIBSSH_LIBRARIES ssh)
   endif()
   add_definitions(-DJOEDB_HAS_SSH)
   message("== ssh was found (${LIBSSH_LIBRARIES})")
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
endif()
