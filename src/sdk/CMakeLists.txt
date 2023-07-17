
function(vision_add_dll name)
    cmake_parse_arguments(DLL "" "" "SOURCES" ${ARGN})
    add_library(${name} MODULE ${DLL_SOURCES})
    target_compile_features(${name} PRIVATE c_std_11 cxx_std_20)
    target_link_libraries(${name} PRIVATE vision-all_libs ${STDCXX_EXTRA_LIBS})
    set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
endfunction()

file(GLOB_RECURSE HEADER_FILES *.h*)
file(GLOB_RECURSE SOURCE_FILES *.c*)

vision_add_dll(vision-renderer SOURCES ${SOURCE_FILES})