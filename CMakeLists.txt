cmake_minimum_required(VERSION 3.0)
project(QuadTargetPlusPlus)

include(UseDoxygen OPTIONAL)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CURRENT_SOURCE_DIR src)

add_executable(QuadTarget src/main.cpp src/TargetFinder_orig.cpp src/TargetFinder_orig.h)
add_executable(QuadTargetStub src/stub.cpp src/TargetFinder.cpp src/TargetFinder.h src/Marker.cpp src/Marker.h src/Target.cpp src/Target.h)
target_link_libraries( QuadTarget ${OpenCV_LIBS} )
target_link_libraries( QuadTargetStub ${OpenCV_LIBS} )
