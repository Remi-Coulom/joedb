project(crazydb)

if(CMAKE_COMPILER_IS_GNUCXX)
 set(CMAKE_CXX_FLAGS "-Wall -ansi -pedantic -Wconversion")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

add_executable(crazydbc
 ../src/compiler/crazydbc.cpp
 ../src/compiler/parse_schema.cpp
)
