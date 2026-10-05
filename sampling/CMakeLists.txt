cmake_minimum_required(VERSION 3.10)

get_filename_component(EXERCISENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

add_executable(${EXERCISENAME} ${SOURCES} ${HEADERS})
if(WIN32)
        target_link_libraries(${EXERCISENAME} "legacy_stdio_definitions.lib")

        #--- Copy the OpenCV DLLs next to the .exe after every build
        # (the "d" versions for Debug, the plain ones for Release)
        add_custom_command(TARGET ${EXERCISENAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CV_DLLS} $<TARGET_FILE_DIR:${EXERCISENAME}>)

        #--- When run from Visual Studio, show only OpenCV errors
        # (hides the harmless INFO messages printed by the Debug libraries)
        set_target_properties(${EXERCISENAME} PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "OPENCV_LOG_LEVEL=ERROR")
endif()
target_link_libraries(${EXERCISENAME} ${COMMON_LIBS})



