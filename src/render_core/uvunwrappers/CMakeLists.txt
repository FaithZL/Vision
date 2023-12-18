add_library(vision-uvunwrapper INTERFACE)

function(vision_add_xatlas_plugin name)
    cmake_parse_arguments(PLUGIN "" "CATEGORY" "SOURCES" ${ARGN})
    set(lib_name vision-${PLUGIN_CATEGORY}-${name})

    add_library(${lib_name} MODULE ${PLUGIN_SOURCES})
    target_link_libraries(${lib_name} PRIVATE vision-base vision-math vision-ext-xatlas)
    target_compile_definitions(${lib_name} PRIVATE VISION_PLUGIN_NAME="${name}")
    set(category vision-${PLUGIN_CATEGORY})
    add_dependencies(${category} ${lib_name})

    set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            WINDOWS_EXPORT_ALL_SYMBOLS ON
            UNITY_BUILD OFF
            DEBUG_POSTFIX "")
endfunction()


vision_add_xatlas_plugin(xatlas CATEGORY uvunwrapper SOURCES xatlas.cpp)