
function(vision_add_plugin name)
    cmake_parse_arguments(PLUGIN "" "CATEGORY" "SOURCES" ${ARGN})
    set(lib_name vision-${PLUGIN_CATEGORY}-${name})

    add_library(${lib_name} MODULE ${PLUGIN_SOURCES})

endfunction()

add_subdirectory(ocarina)
add_subdirectory(ext)
add_subdirectory(description)
add_subdirectory(core)
add_subdirectory(base)
add_subdirectory(math)
add_subdirectory(apps)

add_subdirectory(filters)
add_subdirectory(lights)
add_subdirectory(lightsamplers)
add_subdirectory(integrators)
add_subdirectory(materials)
add_subdirectory(samplers)
add_subdirectory(sensors)
add_subdirectory(textures)
add_subdirectory(tests)

add_library(vision-all INTERFACE)

target_link_libraries(vision-all INTERFACE
        vision-description
        vision-math
        vision-core)