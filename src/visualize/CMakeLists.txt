file(GLOB HEADER_FILES *.h*)
file(GLOB SOURCE_FILES *.c*)

add_library(vision-visualize SHARED ${SOURCE_FILES})
target_link_libraries(vision-visualize PUBLIC
        vision-UI
        vision-hotfix-all
        ocarina
        ocarina-include)

target_compile_definitions(vision-visualize PRIVATE VISION_VISUALIZE_EXPORT_DLL=1)
set_target_properties(vision-visualize PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        WINDOWS_EXPORT_ALL_SYMBOLS ON)