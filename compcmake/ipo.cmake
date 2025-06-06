include_guard(GLOBAL)

#############################################################################
# IPO
#############################################################################
if (CMAKE_BUILD_TYPE MATCHES "Release")
 if (NOT CMAKE_CROSSCOMPILING)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_supported)
 endif()
endif()

if (ipo_supported)
 message("-- IPO supported")
else()
 message("-- IPO not supported")
endif()

function(target_uses_ipo target)
 if (ipo_supported)
  set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
 endif()
endfunction()

function(ipo_add_executable)
 add_executable(${ARGV})
 target_uses_ipo(${ARGV0})
endfunction(ipo_add_executable)
