#############################################################################
if(CMAKE_COMPILER_IS_GNUCXX)
#############################################################################
 message("== gcc")
 set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra -Wno-unused-parameter -pedantic -Wconversion -Wunused-macros -Wcast-qual -Wcast-align -Wparentheses -Wlogical-op -Wmissing-declarations -Wredundant-decls"
 )

 if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wc++14-compat")
 endif()

 if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.4)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicated-cond -Wshadow=local -Wc++17-compat")
 endif()

 if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 11.0)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-stringop-overread") # produces false warnings
 endif()

 set(CMAKE_CXX_FLAGS_COVERAGE
  "-g -O1 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-inline -fno-default-inline -fno-inline-small-functions --coverage"
 )

 set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

 set(CMAKE_CXX_FLAGS_ASAN "-fsanitize=address -O2")
 set(CMAKE_LINKER_FLAGS_ASAN "${CMAKE_CXX_FLAGS_ASAN}")

 set(CMAKE_CXX_FLAGS_TSAN "-fsanitize=thread -O2")
 set(CMAKE_LINKER_FLAGS_TSAN ${CMAKE_CXX_FLAGS_TSAN})

 set(CMAKE_CXX_FLAGS_DEV "-O1 -DNDEBUG")
endif()

#############################################################################
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#############################################################################
 message("== clang")

 find_program(HAS_CLANG_TIDY clang-tidy)
 if (HAS_CLANG_TIDY)
  set(CMAKE_CXX_CLANG_TIDY clang-tidy --header-filter=* -checks=-*,readability-*,-readability-braces-around-statements,-readability-magic-numbers,-readability-implicit-bool-conversion,-readability-else-after-return,-readability-uppercase-literal-suffix,-readability-static-accessed-through-instance,bugprone-*,-bugprone-macro-parentheses,-bugprone-exception-escape,-bugprone-branch-clone,concurrency-*,modernize-*,-modernize-use-trailing-return-type,-modernize-use-auto,-modernize-raw-string-literal,-modernize-avoid-c-arrays,-modernize-deprecated-headers,-modernize-loop-convert,-modernize-return-braced-init-list,-modernize-use-default-member-init,-modernize-use-using,-modernize-concat-nested-namespaces)
 else()
  message("-- no clang-tidy")
 endif()

 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-gnu-zero-variadic-macro-arguments -Wunused-macros -Wcast-qual -Wcast-align -Wparentheses -Wmissing-declarations")

 if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 8.0)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow-field-in-constructor-modified -Wshadow-uncaptured-local -Wshadow -Wshadow-ivar")
 endif()

 set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -fsave-optimization-record")

 # See doc/source/memsan.rst to build libc++ with memsan
 find_path(LLVM_PROJECT_DIR HINTS ${CMAKE_CURRENT_LIST_DIR}/../.. ${CMAKE_CURRENT_LIST_DIR}/../../../../../repos NAMES llvm-project NO_CACHE)
 message("-- CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")
 message("-- LLVM_PROJECT_DIR: ${LLVM_PROJECT_DIR}")
 set(LLVM_PROJECT_BUILD ${LLVM_PROJECT_DIR}/llvm-project/build)

 set(CMAKE_CXX_FLAGS_MSAN "-g -fsanitize=memory -fsanitize-memory-track-origins -O2 -stdlib=libc++ -isystem ${LLVM_PROJECT_BUILD}/include/c++/v1 -L${LLVM_PROJECT_BUILD}/lib -Qunused-arguments -Wl,-rpath,${LLVM_PROJECT_BUILD}/lib")
 set(CMAKE_LINKER_FLAGS_MSAN "${CMAKE_CXX_FLAGS_MSAN} -lc++abi")
endif()

#############################################################################
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#############################################################################
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /w34265 /permissive-") # C4265 virtual destructor
endif()

#############################################################################
if(WIN32)
#############################################################################
 message("== win32")
 add_definitions(-D_CRT_SECURE_NO_WARNINGS)
 add_definitions(-DWIN32_LEAN_AND_MEAN)
 add_definitions(-DNOMINMAX)
endif()
