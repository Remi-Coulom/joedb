#############################################################################
if(CMAKE_COMPILER_IS_GNUCXX)
#############################################################################
 message("== gcc")
 set(CMAKE_CXX_FLAGS
  "-Wall -Wextra -Wno-unused-parameter -pedantic -Wconversion -Wstack-usage=131072 -Wunused-macros -Wc++14-compat -Wcast-qual -Wcast-align -Wparentheses -Wlogical-op -Wmissing-declarations -Wredundant-decls"
 )
 if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.4)
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wduplicated-cond -Wshadow=local -Wc++17-compat")
 endif()
 set(CMAKE_CXX_FLAGS_COVERAGE
  "-g -O0 -fno-inline -fno-default-inline -fno-inline-small-functions --coverage"
 )
endif()

#############################################################################
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#############################################################################
 message("== clang")

 find_program(HAS_CLANG_TIDY clang-tidy)
 if (HAS_CLANG_TIDY)
  set(CMAKE_CXX_CLANG_TIDY clang-tidy --header-filter=* -checks=-*,readability-*,-readability-braces-around-statements,-readability-magic-numbers,-readability-implicit-bool-conversion,-readability-else-after-return,-readability-uppercase-literal-suffix,-readability-static-accessed-through-instance,bugprone-*,-bugprone-macro-parentheses,-bugprone-exception-escape,-bugprone-branch-clone,concurrency-*,modernize-*,-modernize-use-trailing-return-type,-modernize-use-auto,-modernize-raw-string-literal,-modernize-avoid-c-arrays,-modernize-deprecated-headers,-modernize-loop-convert,-modernize-return-braced-init-list,-modernize-use-default-member-init,-modernize-use-using)
 else()
  message("-- no clang-tidy")
 endif()

 set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Wno-unused-parameter -Wunused-macros -Wcast-qual -Wcast-align -Wparentheses -Wmissing-declarations")

 if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 8.0)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow-field-in-constructor-modified -Wshadow-uncaptured-local -Wshadow -Wshadow-ivar")
 endif()

 set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

#############################################################################
if(WIN32)
#############################################################################
 message("== win32")
 add_definitions(-D_CRT_SECURE_NO_WARNINGS)
 add_definitions(-DWIN32_LEAN_AND_MEAN)
 add_definitions(-DNOMINMAX)
endif()
