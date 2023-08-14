
add_library(vision-usd INTERFACE)
target_include_directories(vision-usd
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_directories(vision-usd INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/usd)


file(GLOB_RECURSE LIB_FILES lib/*.lib)

#message(${LIB_FILES})

target_link_libraries(vision-usd INTERFACE
        ${LIB_FILES}
        )