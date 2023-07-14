add_library(vision-loader INTERFACE)
add_subdirectory(assimp)
add_subdirectory(json)
add_subdirectory(usd)

target_link_libraries(vision-loader INTERFACE
        vision-json
        vision-usd
        vision-assimp)
