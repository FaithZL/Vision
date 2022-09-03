

add_subdirectory(ocarina)
add_subdirectory(apps)
add_library(vision-all INTERFACE)

target_link_libraries(vision-all INTERFACE
        ocarina)