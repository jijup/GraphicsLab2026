# Build configuration file for "Computer Graphics 2026 Lab"
#
#--- To understand its content:
#   https://cmake.org/cmake/help/latest/manual/cmake-language.7.html
#   https://cmake.org/cmake/help/latest/manual/cmake-variables.7.html
#

#--- This is how you show a status message in the build system
message(STATUS "Computer Graphics Lab 2026 - Loading Common Configuration")

#--- Tell CMake it can include ".cmake" configuration
# files from the folder where the current file is located
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

#--- Common headers/libraries for all the exercises
include_directories(${CMAKE_CURRENT_LIST_DIR})

#--- Make headers in common directory visible in the IDE
file(GLOB_RECURSE COMMON_DIR_HEADERS "${CMAKE_CURRENT_LIST_DIR}/*.h")
add_custom_target(common_headers SOURCES ${COMMON_DIR_HEADERS})