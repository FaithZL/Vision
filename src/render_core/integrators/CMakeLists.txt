add_library(vision-integrator INTERFACE)

add_subdirectory(ReSTIR)

vision_add_plugin(pt CATEGORY integrator SOURCES pt.cpp)
vision_add_plugin(ao CATEGORY integrator SOURCES ao.cpp)

function(vision_add_rt_plugin name)
    cmake_parse_arguments(PLUGIN "" "CATEGORY" "SOURCES" ${ARGN})
    set(lib_name vision-${PLUGIN_CATEGORY}-${name})

    add_library(${lib_name} MODULE ${PLUGIN_SOURCES})
    target_link_libraries(${lib_name} PRIVATE vision-base vision-math vision-integrator-ReSTIR)
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

vision_add_rt_plugin(rt CATEGORY integrator SOURCES rt.cpp)