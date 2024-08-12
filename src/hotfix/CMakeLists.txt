file(GLOB_RECURSE HEADER_FILES *.h*)
file(GLOB_RECURSE SOURCE_FILES *.c*)

add_library(vision-hotfix SHARED ${SOURCE_FILES})
target_link_libraries(vision-hotfix PUBLIC
        ocarina
        ocarina-include)

target_compile_definitions(vision-hotfix PRIVATE VISION_HOTFIX_EXPORT_DLL=1 -DCMAKE_VERBOSE_MAKEFILE=ON)
set_target_properties(vision-hotfix PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        WINDOWS_EXPORT_ALL_SYMBOLS ON)