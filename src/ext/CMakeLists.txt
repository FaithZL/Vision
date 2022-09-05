add_subdirectory(cxxopts)
target_include_directories(cxxopts INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cxxopts/include)

add_library(vision-ext INTERFACE)
target_link_libraries(vision-ext INTERFACE
        cxxopts)
