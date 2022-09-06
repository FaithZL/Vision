

add_subdirectory(ocarina)
add_subdirectory(ext)
add_subdirectory(core)
add_subdirectory(loader)
add_subdirectory(apps)
add_library(vision-all INTERFACE)

target_link_libraries(vision-all INTERFACE
        vision-core)