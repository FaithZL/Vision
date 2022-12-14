
function(vision_add_plugin name)
    cmake_parse_arguments(PLUGIN "" "CATEGORY" "SOURCES" ${ARGN})
    set(lib_name vision-${PLUGIN_CATEGORY}-${name})

    add_library(${lib_name} MODULE ${PLUGIN_SOURCES})
    target_link_libraries(${lib_name} PRIVATE vision-base vision-math)
    set(category vision-${PLUGIN_CATEGORY})
    add_dependencies(${category} ${lib_name})

    set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            WINDOWS_EXPORT_ALL_SYMBOLS ON
            UNITY_BUILD OFF
            DEBUG_POSTFIX "")
endfunction()

add_subdirectory(ocarina)
add_subdirectory(ext)
add_subdirectory(descriptions)
add_subdirectory(core)
add_subdirectory(base)
add_subdirectory(math)
add_subdirectory(apps)

add_subdirectory(plugins)

add_subdirectory(tests)


add_library(vision-main INTERFACE)
target_link_libraries(vision-main INTERFACE
        vision-description
        vision-base
        vision-math
        vision-core)

add_library(vision-all INTERFACE)
target_link_libraries(vision-all INTERFACE
        vision-main
        vision-plugins)