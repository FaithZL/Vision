
function(vision_add_test name)
    cmake_parse_arguments(TEST "" "" "SOURCES" ${ARGN})
    add_executable(${name}  ${TEST_SOURCES})
    target_link_libraries(${name} vision-all_libs)
    set_target_properties(${name} PROPERTIES
            PUBLIC_HEADER "${HEADER_FILES}"
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            WINDOWS_EXPORT_ALL_SYMBOLS ON)
endfunction()

vision_add_test(test-playground SOURCES test_playground.cpp)
vision_add_test(test-spawn_ray SOURCES test_spawn_ray.cpp)
vision_add_test(test-bxdf SOURCES test_bxdf.cpp)
vision_add_test(test-warper2d SOURCES test_warper2d.cpp)
vision_add_test(test-ies SOURCES test_ies.cpp)
vision_add_test(test-render_graph SOURCES test_render_graph.cpp)

function(vision_add_simple_test name)
    cmake_parse_arguments(TEST "" "" "SOURCES" ${ARGN})
    add_executable(${name}  ${TEST_SOURCES})
    target_link_libraries(${name} vision-hotfix)
    set_target_properties(${name} PROPERTIES
            PUBLIC_HEADER "${HEADER_FILES}"
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            WINDOWS_EXPORT_ALL_SYMBOLS ON)
endfunction()

vision_add_simple_test(vision-test_hotfix SOURCES test_hotfix.cpp)

add_executable(vision-test_sdk test_sdk.cpp)
add_dependencies(vision-test_sdk vision-renderer)
