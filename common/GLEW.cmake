# GLEW (statically linked). Uses FindGLEW.cmake in this folder.
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
add_definitions(-DWITH_GLEW)
if(WIN32)
    add_definitions(-DGLEW_STATIC)   # the static glew library in external/glew
endif()
list(APPEND COMMON_LIBS ${GLEW_LIBRARIES})
