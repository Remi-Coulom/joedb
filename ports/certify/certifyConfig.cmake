include(CMakeFindDependencyMacro)

find_dependency(Boost CONFIG)
find_dependency(OpenSSL)

if(NOT TARGET certify::core)
 get_filename_component(certify_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

 add_library(certify::core INTERFACE IMPORTED)
 set_target_properties(certify::core PROPERTIES
  INTERFACE_COMPILE_FEATURES cxx_std_11
  INTERFACE_INCLUDE_DIRECTORIES "${certify_ROOT}/include"
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

set(certify_FOUND TRUE)
