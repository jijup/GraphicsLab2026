# Locate GLFW 3. Defines:
#   GLFW3_INCLUDE_DIRS, where to find GLFW/glfw3.h
#   GLFW3_LIBRARIES,    the library to link
#   GLFW3_FOUND,        true if both were found
#
# Windows:        external/glfw (include, lib-vc2019 or lib-vc2022)
# Linux / macOS:  the GLFW installed on the machine

if(MSVC)
    #--- Visual Studio 2019 is MSVC_VERSION 1920 to 1929, 2022 is 1930 and up
    if(MSVC_VERSION GREATER_EQUAL 1930)
        set(GLFW_WINLIBS lib-vc2022 lib-vc2019)
    else()
        set(GLFW_WINLIBS lib-vc2019 lib-vc2022)
    endif()
    #--- glfw3_mt uses the static C++ runtime (/MT), which OpenCV.cmake sets for
    # every exercise. Plain glfw3 uses /MD and fails to link with /MT.
    set(GLFW_LIBNAMES glfw3_mt)
elseif(MINGW)
    set(GLFW_WINLIBS lib-mingw-w64)
    set(GLFW_LIBNAMES glfw3)
else()
    set(GLFW_LIBNAMES glfw3 glfw)
endif()

set(GLFW_WINPATHS "")
foreach(d ${GLFW_WINLIBS})
    list(APPEND GLFW_WINPATHS ${CMAKE_SOURCE_DIR}/external/glfw/${d})
endforeach()

find_path(GLFW3_INCLUDE_DIRS GLFW/glfw3.h
    PATHS
    ${CMAKE_SOURCE_DIR}/external/glfw/include
    $ENV{GLFWDIR}/include
    /usr/local/include
    /opt/homebrew/include
    /usr/include
    /opt/include)

find_library(GLFW3_LIBRARIES
    NAMES ${GLFW_LIBNAMES}
    PATHS
    ${GLFW_WINPATHS}
    $ENV{GLFWDIR}/lib
    /usr/local/lib
    /opt/homebrew/lib
    /usr/lib
    /usr/lib/x86_64-linux-gnu
    /opt/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW3 DEFAULT_MSG GLFW3_LIBRARIES GLFW3_INCLUDE_DIRS)
