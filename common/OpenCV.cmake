# OpenCV for the lab exercises. Adds the OpenCV libraries to COMMON_LIBS.
#
# Windows:        the pre-built copy in external/OpenCV (include, lib, dll)
# Linux / macOS:  the OpenCV installed on the machine
#                   Ubuntu/Debian:  sudo apt install libopencv-dev
#                   macOS:          brew install opencv

if(WIN32)
    set(OPENCV_DIR ${CMAKE_SOURCE_DIR}/external/OpenCV)
    include_directories(${OPENCV_DIR}/include)

    #--- Use the static C++ runtime (/MT, /MTd) to match the pre-built libraries.
    # Only the runtime flag is swapped. Release keeps its optimisation (/O2)
    # and Debug keeps its debug information (/Zi).
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")   # CMake 3.15+
    foreach(flags CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                  CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
        string(REPLACE "/MD" "/MT" ${flags} "${${flags}}")                      # older CMake
    endforeach()

    #--- Libraries: release (optimized) and debug versions of each module
    foreach(module core imgcodecs imgproc videoio highgui)
        list(APPEND COMMON_LIBS
            optimized ${OPENCV_DIR}/lib/opencv_${module}453.lib
            debug     ${OPENCV_DIR}/lib/opencv_${module}453d.lib)
    endforeach()
    list(APPEND COMMON_LIBS
        optimized ${OPENCV_DIR}/lib/zlib.lib
        debug     ${OPENCV_DIR}/lib/zlibd.lib)

    #--- DLLs the program needs at run time.
    # Debug builds need the "d" versions (opencv_core453d.dll),
    # Release builds the plain ones (opencv_core453.dll).
    set(OPENCV_DLL_DIR ${OPENCV_DIR}/dll)
    set(CV_DLLS "")
    foreach(module core imgcodecs imgproc videoio highgui)
        list(APPEND CV_DLLS ${OPENCV_DLL_DIR}/opencv_${module}453$<$<CONFIG:Debug>:d>.dll)
    endforeach()

else()
    #--- Linux and macOS. CONFIG uses the OpenCVConfig.cmake that comes with
    # OpenCV itself (and ignores any old FindOpenCV.cmake in common/).
    find_package(OpenCV REQUIRED CONFIG
        COMPONENTS core imgcodecs imgproc videoio highgui)
    include_directories(${OpenCV_INCLUDE_DIRS})
    list(APPEND COMMON_LIBS ${OpenCV_LIBS})
endif()

add_definitions(-DWITH_OPENCV)