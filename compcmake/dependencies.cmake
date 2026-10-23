include_guard(GLOBAL)

# Optional dependencies default to AUTO to preserve the traditional source-build
# behaviour. Package managers should pass ON or OFF explicitly so that the
# resulting joedb feature set does not depend on the build machine.
function(joedb_dependency_option name description)
 set(${name} "AUTO" CACHE STRING "${description} (AUTO, ON, or OFF)")
 set_property(CACHE ${name} PROPERTY STRINGS AUTO ON OFF)
 string(TOUPPER "${${name}}" value)
 if(NOT value MATCHES "^(AUTO|ON|OFF)$")
  message(FATAL_ERROR "${name} must be AUTO, ON, or OFF (got '${${name}}')")
 endif()
 set(${name} "${value}" CACHE STRING "${description} (AUTO, ON, or OFF)" FORCE)
endfunction()

joedb_dependency_option(JOEDB_WITH_ASIO "Build Boost.Asio support")
joedb_dependency_option(JOEDB_WITH_WEBSOCKETS "Build WebSocket support")
joedb_dependency_option(JOEDB_WITH_SSH "Build libssh support")

if(JOEDB_NO_CURL AND NOT DEFINED JOEDB_WITH_CURL)
 set(JOEDB_WITH_CURL OFF)
endif()
joedb_dependency_option(JOEDB_WITH_CURL "Build libcurl support")
joedb_dependency_option(JOEDB_WITH_BROTLI "Build Brotli support")

message("-- UNIX = \"${UNIX}\"")
message("-- CMAKE_CROSSCOMPILING = \"${CMAKE_CROSSCOMPILING}\"")
message("-- CMAKE_SYSTEM_NAME = \"${CMAKE_SYSTEM_NAME}\"")
message("-- CMAKE_SIZEOF_VOID_P = \"${CMAKE_SIZEOF_VOID_P}\"")

set(JOEDB_HAS_BOOST FALSE)
set(JOEDB_HAS_ASIO FALSE)
set(JOEDB_HAS_WEBSOCKETS FALSE)
set(JOEDB_HAS_SSH FALSE)
set(JOEDB_HAS_CURL FALSE)
set(JOEDB_HAS_BROTLI FALSE)
set(JOEDB_BROTLI_PROVIDER "")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
set(JOEDB_EXTERNAL_LIBS Threads::Threads)

#############################################################################
# Boost.Asio and WebSockets
#############################################################################
if(NOT "${JOEDB_WITH_ASIO}" STREQUAL "OFF" OR
   NOT "${JOEDB_WITH_WEBSOCKETS}" STREQUAL "OFF")
 if("${JOEDB_WITH_ASIO}" STREQUAL "ON" OR
    "${JOEDB_WITH_WEBSOCKETS}" STREQUAL "ON")
  find_package(Boost CONFIG REQUIRED)
 else()
  find_package(Boost CONFIG QUIET)
 endif()
endif()

if(NOT "${JOEDB_WITH_ASIO}" STREQUAL "OFF" AND Boost_FOUND)
 find_path(JOEDB_ASIO_INCLUDE_DIR boost/asio.hpp HINTS ${Boost_INCLUDE_DIRS})
 if(JOEDB_ASIO_INCLUDE_DIR)
  set(JOEDB_HAS_ASIO TRUE)
 elseif("${JOEDB_WITH_ASIO}" STREQUAL "ON")
  message(FATAL_ERROR "JOEDB_WITH_ASIO=ON, but boost/asio.hpp was not found")
 endif()
endif()

if(NOT "${JOEDB_WITH_WEBSOCKETS}" STREQUAL "OFF" AND Boost_FOUND)
 find_path(JOEDB_BEAST_INCLUDE_DIR boost/beast/websocket.hpp HINTS ${Boost_INCLUDE_DIRS})

 if("${JOEDB_WITH_WEBSOCKETS}" STREQUAL "ON")
  find_package(OpenSSL REQUIRED)
 else()
  find_package(OpenSSL QUIET)
 endif()

 if(OpenSSL_FOUND)
  find_package(certify CONFIG QUIET)

  if(NOT TARGET certify::core)
   find_path(
    JOEDB_CERTIFY_INCLUDE_DIR
    boost/certify/https_verification.hpp
    HINTS "${CMAKE_CURRENT_LIST_DIR}/../../certify/include"
   )

   if(JOEDB_CERTIFY_INCLUDE_DIR)
    message("-- Using Certify headers from ${JOEDB_CERTIFY_INCLUDE_DIR}")
    add_library(certify::core INTERFACE IMPORTED GLOBAL)
    set_target_properties(certify::core PROPERTIES
     INTERFACE_COMPILE_FEATURES cxx_std_11
     INTERFACE_INCLUDE_DIRECTORIES "${JOEDB_CERTIFY_INCLUDE_DIR}"
     INTERFACE_LINK_LIBRARIES "Boost::headers;OpenSSL::SSL;OpenSSL::Crypto"
    )

    if(WIN32)
     set_property(TARGET certify::core APPEND PROPERTY
      INTERFACE_LINK_LIBRARIES Crypt32.lib)
    elseif(APPLE)
     set_property(TARGET certify::core APPEND PROPERTY
      INTERFACE_LINK_LIBRARIES "-framework CoreFoundation;-framework Security")
    endif()
   endif()
  endif()
 endif()

 if(JOEDB_BEAST_INCLUDE_DIR AND OpenSSL_FOUND AND TARGET certify::core)
  set(JOEDB_HAS_WEBSOCKETS TRUE)
 elseif("${JOEDB_WITH_WEBSOCKETS}" STREQUAL "ON")
  message(FATAL_ERROR
   "JOEDB_WITH_WEBSOCKETS=ON requires Boost.Beast, Certify, and OpenSSL"
  )
 endif()
endif()

if(JOEDB_HAS_ASIO OR JOEDB_HAS_WEBSOCKETS)
 set(JOEDB_HAS_BOOST TRUE)
 list(APPEND JOEDB_EXTERNAL_LIBS Boost::headers)
 add_compile_definitions(JOEDB_HAS_BOOST)
endif()

if(JOEDB_HAS_ASIO)
 message("-- Boost.Asio support enabled")
 add_compile_definitions(JOEDB_HAS_ASIO)
 if(WIN32)
  list(APPEND JOEDB_EXTERNAL_LIBS wsock32 ws2_32)
 endif()
else()
 message("-- Boost.Asio support disabled")
endif()

if(JOEDB_HAS_WEBSOCKETS)
 message("-- WebSocket support enabled")
 add_compile_definitions(JOEDB_HAS_WEBSOCKETS)
 list(APPEND JOEDB_EXTERNAL_LIBS certify::core OpenSSL::SSL OpenSSL::Crypto)
else()
 message("-- WebSocket support disabled")
endif()

#############################################################################
# libssh
#############################################################################
if(NOT "${JOEDB_WITH_SSH}" STREQUAL "OFF")
 find_package(libssh CONFIG QUIET)
 if(NOT libssh_FOUND)
  find_package(libssh QUIET)
 endif()

 if(libssh_FOUND)
  set(JOEDB_HAS_SSH TRUE)
 elseif("${JOEDB_WITH_SSH}" STREQUAL "ON")
  message(FATAL_ERROR "JOEDB_WITH_SSH=ON, but libssh was not found")
 endif()
endif()

if(JOEDB_HAS_SSH)
 message("-- libssh support enabled")
 add_compile_definitions(JOEDB_HAS_SSH)
 list(APPEND JOEDB_EXTERNAL_LIBS ssh)
else()
 message("-- libssh support disabled")
endif()

#############################################################################
# libcurl
#############################################################################
if(NOT "${JOEDB_WITH_CURL}" STREQUAL "OFF")
 if("${JOEDB_WITH_CURL}" STREQUAL "ON")
  find_package(CURL REQUIRED)
 else()
  find_package(CURL QUIET)
 endif()

 if(CURL_FOUND)
  set(JOEDB_HAS_CURL TRUE)
 endif()
endif()

if(JOEDB_HAS_CURL)
 message("-- libcurl support enabled")
 add_compile_definitions(JOEDB_HAS_CURL)
 list(APPEND JOEDB_EXTERNAL_LIBS CURL::libcurl)
else()
 message("-- libcurl support disabled")
endif()

#############################################################################
# Brotli
#############################################################################
if(NOT "${JOEDB_WITH_BROTLI}" STREQUAL "OFF")
 find_package(unofficial-brotli CONFIG QUIET)
 if(unofficial-brotli_FOUND)
  set(JOEDB_HAS_BROTLI TRUE)
  set(JOEDB_BROTLI_PROVIDER "unofficial-brotli")
  list(APPEND JOEDB_EXTERNAL_LIBS
   unofficial::brotli::brotlidec
   unofficial::brotli::brotlienc
  )
 else()
  set(joedb_saved_module_path "${CMAKE_MODULE_PATH}")
  list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
  find_package(Brotli QUIET)
  set(CMAKE_MODULE_PATH "${joedb_saved_module_path}")
  unset(joedb_saved_module_path)

  if(Brotli_FOUND)
   set(JOEDB_HAS_BROTLI TRUE)
   set(JOEDB_BROTLI_PROVIDER "Brotli")
   list(APPEND JOEDB_EXTERNAL_LIBS Brotli::decoder Brotli::encoder)
  elseif("${JOEDB_WITH_BROTLI}" STREQUAL "ON")
   message(FATAL_ERROR "JOEDB_WITH_BROTLI=ON, but Brotli was not found")
  endif()
 endif()
endif()

if(JOEDB_HAS_BROTLI)
 message("-- Brotli support enabled (${JOEDB_BROTLI_PROVIDER})")
 add_compile_definitions(JOEDB_HAS_BROTLI)
else()
 message("-- Brotli support disabled")
endif()

#############################################################################
# Platform libraries
#############################################################################
if(ANDROID)
 list(APPEND JOEDB_EXTERNAL_LIBS log)
elseif(APPLE)
 list(APPEND JOEDB_EXTERNAL_LIBS "-framework OSLog")
endif()
