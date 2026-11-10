cmake_minimum_required(VERSION 3.10)

project(GraphicsLab2026)

#--- Load the common configuration
include(common/config.cmake)

#--- Load third party packages
include(common/Eigen.cmake)
include(common/OpenCV.cmake)

#--- OpenGL configuration
include(common/OpenGL.cmake)
include(common/GLEW.cmake)
include(common/GLFW.cmake)

#Assignments
#add_subdirectory(cornellbox)

#--- Subprojects
#add_subdirectory(hello)
#add_subdirectory(vectors)
#add_subdirectory(raytrace)
#add_subdirectory(sampling)
#add_subdirectory(bumpmapping)
#add_subdirectory(transformations)
#add_subdirectory(shaders)
add_subdirectory(virtuallandscape)

#--- C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)









