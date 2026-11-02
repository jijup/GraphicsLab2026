# GLFW 3. Uses FindGLFW3.cmake in this folder.
find_package(GLFW3 REQUIRED)
include_directories(${GLFW3_INCLUDE_DIRS})
add_definitions(-DWITH_GLFW)
list(APPEND COMMON_LIBS ${GLFW3_LIBRARIES})

#--- On macOS, GLFW needs these system frameworks
if(APPLE)
    list(APPEND COMMON_LIBS "-framework Cocoa" "-framework IOKit")
endif()
