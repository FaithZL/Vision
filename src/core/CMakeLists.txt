file(GLOB_RECURSE HEADER_FILES *.h*)
file(GLOB_RECURSE SOURCE_FILES *.c*)

add_library(vision-core SHARED ${SOURCE_FILES})
target_link_libraries(vision-core PUBLIC
        ocarina
        vision-ext
        vision-base
        vision-description
        ocarina-include)

target_compile_definitions(vision-core PRIVATE VISION_CORE_EXPORT_DLL=1)
set_target_properties(vision-core PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        WINDOWS_EXPORT_ALL_SYMBOLS ON)