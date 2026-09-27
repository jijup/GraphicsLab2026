# Finds Eigen 3 (header-only) on Windows, Linux and macOS.
#
# Sets:
#   EIGEN3_FOUND          TRUE if Eigen was found
#   EIGEN3_INCLUDE_DIRS   the folder to add to the include path

# 1. The copy bundled with the course repo, so everyone builds against the same version.
find_path(EIGEN3_INCLUDE_DIR
    NAMES Eigen/Geometry
    PATHS ${CMAKE_SOURCE_DIR}/external/eigen/include
    NO_DEFAULT_PATH)

# 2. Otherwise, a copy installed on the machine.
#    The EIGEN3DIR environment variable, then the usual system locations.
#    Package managers put Eigen in an "eigen3" subfolder (apt, Homebrew, MacPorts),
#    which PATH_SUFFIXES handles.
find_path(EIGEN3_INCLUDE_DIR
    NAMES Eigen/Geometry
    HINTS
        $ENV{EIGEN3DIR}/include
        $ENV{EIGEN3DIR}
    PATHS
        /usr/local/include        # Linux, Intel Macs (Homebrew)
        /usr/include              # Linux (apt, dnf)
        /opt/homebrew/include     # Apple Silicon Macs (Homebrew)
        /opt/local/include        # macOS (MacPorts)
    PATH_SUFFIXES eigen3)

# Sets EIGEN3_FOUND, prints where Eigen was found, and stops with a clear
# error if find_package(Eigen3 REQUIRED) was used and nothing was found.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Eigen3 DEFAULT_MSG EIGEN3_INCLUDE_DIR)

if(EIGEN3_FOUND)
    set(EIGEN3_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR})
endif()

mark_as_advanced(EIGEN3_INCLUDE_DIR)
