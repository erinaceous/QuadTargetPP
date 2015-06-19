cmake_minimum_required(VERSION 3.2)
project(QuadTargetPlusPlus)

include(UseDoxygen OPTIONAL)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CURRENT_SOURCE_DIR src)

set(SOURCE_FILES src/main.cpp)
add_executable(QuadTarget__ ${SOURCE_FILES})
