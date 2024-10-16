add_library(vision-integrator INTERFACE)

add_subdirectory(ReSTIR)
add_subdirectory(surfel)
add_subdirectory(adaptive)

function(vision_add_pt_plugin name)
    cmake_parse_arguments(PLUGIN "" "CATEGORY" "SOURCES;DEPENDENCE" ${ARGN})
    set(lib_name vision-${PLUGIN_CATEGORY}-${name})

    add_library(${lib_name} MODULE ${PLUGIN_SOURCES})
    target_link_libraries(${lib_name} PRIVATE vision-base
            vision-math
            ${PLUGIN_DEPENDENCE})

    target_compile_definitions(${lib_name} PRIVATE VISION_PLUGIN_NAME="${name}"  VISION_CATEGORY="${PLUGIN_CATEGORY}")
    set(category vision-${PLUGIN_CATEGORY})
    add_dependencies(${category} ${lib_name})

    set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            WINDOWS_EXPORT_ALL_SYMBOLS ON
            UNITY_BUILD OFF
            DEBUG_POSTFIX "")
endfunction()

vision_add_pt_plugin(pt CATEGORY integrator DEPENDENCE "vision-integrator-adaptive" SOURCES pt.cpp )
vision_add_pt_plugin(rt CATEGORY integrator DEPENDENCE "vision-integrator-surfel;vision-integrator-ReSTIR"  SOURCES rt.cpp)

vision_add_plugin(ao CATEGORY integrator SOURCES ao.cpp)

