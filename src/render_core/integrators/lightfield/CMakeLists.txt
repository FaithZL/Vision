file(GLOB_RECURSE SOURCE_FILES *.c*)
file(GLOB_RECURSE HEADER_FILES *.h*)

add_library(vision-integrator-lightfield SHARED ${SOURCE_FILES})
target_link_libraries(vision-integrator-lightfield PUBLIC
        vision-base vision-math)

set_target_properties(vision-integrator-lightfield PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        WINDOWS_EXPORT_ALL_SYMBOLS ON)