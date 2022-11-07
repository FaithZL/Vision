add_library(vision-shapes INTERFACE)
vision_add_plugin(quad CATEGORY shape SOURCES quad.cpp)
vision_add_plugin(cube CATEGORY shape SOURCES cube.cpp)
vision_add_plugin(sphere CATEGORY shape SOURCES sphere.cpp)
function(vision_add_model_plugin name)
    cmake_parse_arguments(PLUGIN "" "CATEGORY" "SOURCES" ${ARGN})
    set(lib_name vision-${PLUGIN_CATEGORY}-${name})

    add_library(${lib_name} MODULE ${PLUGIN_SOURCES})
    target_link_libraries(${lib_name} PRIVATE vision-main vision-ext-assimp_config)
    set(category vision-${PLUGIN_CATEGORY}s)
    add_dependencies(${category} ${lib_name})

    set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            WINDOWS_EXPORT_ALL_SYMBOLS ON
            UNITY_BUILD OFF
            DEBUG_POSTFIX "")
endfunction()
vision_add_model_plugin(model CATEGORY shape SOURCES model.cpp)

