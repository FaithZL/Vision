
function(vision_add_plugin name)
    cmake_parse_arguments(PLUGIN "" "CATEGORY" "SOURCES;DEPENDENCE" ${ARGN})
    set(lib_name vision-${PLUGIN_CATEGORY}-${name})

    add_library(${lib_name} MODULE ${PLUGIN_SOURCES})
    target_link_libraries(${lib_name} PRIVATE
            vision-base
            vision-math
            ${PLUGIN_DEPENDENCE})

    target_compile_definitions(${lib_name} PRIVATE
            VISION_PLUGIN_NAME="${name}"
            VISION_CATEGORY="${PLUGIN_CATEGORY}")
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
add_subdirectory(core)
add_subdirectory(UI)
add_subdirectory(base)
add_subdirectory(math)
add_subdirectory(apps)
add_subdirectory(render_graph)
add_subdirectory(importers)
add_subdirectory(pipelines)
add_subdirectory(render_core)
add_subdirectory(hotfix)
add_subdirectory(sdk)

if (${OCARINA_BUILD_TESTS})
    add_subdirectory(tests)
endif ()


add_library(vision-main INTERFACE)
target_link_libraries(vision-main INTERFACE
        vision-core
        vision-base
        vision-importer
        vision-pipeline
        vision-render_graph
        vision-math)

add_library(vision-all_libs INTERFACE)
target_link_libraries(vision-all_libs INTERFACE
        vision-main
        vision-plugins)

if (${OCARINA_PY})
    add_subdirectory(pyexporter)
endif ()