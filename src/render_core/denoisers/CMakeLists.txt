add_library(vision-denoiser INTERFACE)

function(vision_add_oidn_plugin name)
    cmake_parse_arguments(PLUGIN "" "CATEGORY" "SOURCES" ${ARGN})
    set(lib_name vision-${PLUGIN_CATEGORY}-${name})

    add_library(${lib_name} MODULE ${PLUGIN_SOURCES})
    target_link_libraries(${lib_name} PRIVATE vision-main vision-ext-oidn)
    target_compile_definitions(${lib_name} PRIVATE VISION_PLUGIN_NAME="${name}")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/oidn/include)
    set(category vision-${PLUGIN_CATEGORY})
    add_dependencies(${category} ${lib_name})

    set_target_properties(${lib_name} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            WINDOWS_EXPORT_ALL_SYMBOLS ON
            UNITY_BUILD OFF
            DEBUG_POSTFIX "")
endfunction()

vision_add_oidn_plugin(oidn CATEGORY denoiser SOURCES oidn.cpp)
vision_add_plugin(optix CATEGORY denoiser SOURCES optix.cpp)
add_subdirectory(SVGF)