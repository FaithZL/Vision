add_subdirectory(cxxopts)
target_include_directories(cxxopts INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cxxopts/include)

set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT ON CACHE BOOL "" FORCE)

if (MSVC)
    add_compile_options(
            /wd4819
    )

endif ()

add_subdirectory(assimp)
set_target_properties(assimp PROPERTIES UNITY_BUILD OFF)
if (UNIX AND NOT APPLE)
    target_compile_definitions(assimp PRIVATE USE_FILE32API=1)
endif ()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
        CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0)
    target_compile_options(assimp PRIVATE -Wno-deprecated-non-prototype)
endif ()
add_library(vision-ext-assimp_config INTERFACE)
target_link_libraries(vision-ext-assimp_config INTERFACE assimp::assimp)

set(OIDN_DEVICE_CUDA ON)

add_subdirectory(oidn)

set_target_properties(OpenImageDenoise
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/bin/
)

set_target_properties(OpenImageDenoise_core
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/bin/
)

set_target_properties(OpenImageDenoise_device_cpu
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/bin/
)

set_target_properties(OpenImageDenoise_device_cuda
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/bin/
)

add_library(vision-ext-oidn INTERFACE)
target_link_libraries(vision-ext-oidn INTERFACE OpenImageDenoise)

add_subdirectory(xatlas)
add_library(vision-ext-xatlas INTERFACE)
target_link_libraries(vision-ext-xatlas INTERFACE xatlas)

add_library(vision-ext INTERFACE)
target_link_libraries(vision-ext INTERFACE
        cxxopts)

