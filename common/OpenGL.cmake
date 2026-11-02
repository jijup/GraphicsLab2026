# OpenGL for the shader exercises. Adds it to COMMON_LIBS.
# OpenGL comes with the graphics driver on every platform, so nothing is in external/.
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
add_definitions(-DWITH_OPENGL)
list(APPEND COMMON_LIBS ${OPENGL_LIBRARIES})

#--- macOS: OpenGL still works but prints deprecation warnings everywhere
if(APPLE)
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif()
