# Locate GLEW. Defines:
#   GLEW_INCLUDE_DIRS, where to find GL/glew.h
#   GLEW_LIBRARIES,    the library to link
#   GLEW_FOUND,        true if both were found
#
# Windows:        external/glew (include, lib/x64/glew64.lib or glew32s.lib)
# Linux / macOS:  the GLEW installed on the machine

find_path(GLEW_INCLUDE_DIRS GL/glew.h
    PATHS
    ${CMAKE_SOURCE_DIR}/external/glew/include
    $ENV{GLEWDIR}/include
    /usr/local/include
    /opt/homebrew/include
    /usr/include
    /opt/include)

if(WIN32)
    #--- Static libraries only (no DLL to copy). glew64 is the name used in this course.
    set(GLEWLIBNAMES glew64 glew32s)
    set(WINLIBPATHS
        ${CMAKE_SOURCE_DIR}/external/glew/lib/x64
        ${CMAKE_SOURCE_DIR}/external/glew/lib/Release/x64)
else()
    # IMPORTANT: uppercase, otherwise it is not found on Linux
    set(GLEWLIBNAMES GLEW)
endif()

find_library(GLEW_LIBRARIES
    NAMES ${GLEWLIBNAMES}
    PATHS
    ${WINLIBPATHS}
    $ENV{GLEWDIR}/lib
    /usr/local/lib
    /opt/homebrew/lib
    /usr/lib
    /usr/lib/x86_64-linux-gnu
    /opt/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW DEFAULT_MSG GLEW_LIBRARIES GLEW_INCLUDE_DIRS)
