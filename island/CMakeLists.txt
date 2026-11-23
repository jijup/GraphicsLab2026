get_filename_component(EXERCISENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE SHADERS "*.glsl")

add_executable(${EXERCISENAME} ${SOURCES} ${HEADERS} ${SHADERS})
target_link_libraries(${EXERCISENAME} ${COMMON_LIBS})

#--- So "Helpers/ShaderHelper.h" is found from every subfolder (GCC and Clang need this)
target_include_directories(${EXERCISENAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

#--- The .exe goes in build/<exercise>/Release (or Debug), on every platform
set(RUN_DIR ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
set_target_properties(${EXERCISENAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${RUN_DIR}
    VS_DEBUGGER_WORKING_DIRECTORY ${RUN_DIR})

#--- The shaders and textures must sit next to the .exe.
# This runs on every build, so edits to the .glsl files are always picked up.
add_custom_target(${EXERCISENAME}_data ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Shaders ${RUN_DIR}/Shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Textures ${RUN_DIR}/Textures)
add_dependencies(${EXERCISENAME} ${EXERCISENAME}_data)
