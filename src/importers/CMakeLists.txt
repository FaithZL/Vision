add_library(vision-importer INTERFACE)
add_subdirectory(assimp)
add_subdirectory(json)
add_subdirectory(usd)

file(GLOB HEADER_FILES *.h*)
file(GLOB SOURCE_FILES *.c*)

add_library(vision-assimp_util SHARED ${SOURCE_FILES})
target_link_libraries(vision-assimp_util PUBLIC
        vision-base
        vision-ext-assimp_config
        ocarina-include)

set_target_properties(vision-assimp_util PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        WINDOWS_EXPORT_ALL_SYMBOLS ON)