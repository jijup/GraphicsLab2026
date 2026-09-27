# REQUIRED stops the configure step with an error if Eigen is not found.
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIRS})
add_definitions(-DWITH_EIGEN)
