set(CMAKE_CXX_COMPILER g++)

cmake_minimum_required (VERSION 2.8)
project(krechy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_FLAGS_OPTIMIZEDWITHASSERT "${CMAKE_CXX_FLAGS} -O3")

project(root)
include_directories(${root_SOURCE_DIR}/include)

add_library(splits rules/splits.cpp)

add_subdirectory(rules)
add_subdirectory(tests)

set(COMMON_LIBRARIES splits)

message(STATUS "common libraries " ${COMMON_LIBRARIES})

add_executable(sanity tests/sanity.cpp)
target_link_libraries(sanity ${COMMON_LIBRARIES})


add_definitions(-Wall)
