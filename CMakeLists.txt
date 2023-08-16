
cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include_directories(src)

project(Vision)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/")

set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

#set(OCARINA_BUILD_TESTS FALSE)


function(vision_add_application name)
    cmake_parse_arguments(APP "" "" "SOURCES" ${ARGN})
    add_executable(${name} ${APP_SOURCES})
    target_compile_features(${name} PRIVATE c_std_11 cxx_std_20)
    target_link_libraries(${name} PRIVATE vision-all_libs ${STDCXX_EXTRA_LIBS})
    set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
endfunction()

add_subdirectory(src)

add_custom_target(CP_OIDN_CUDA ALL
        COMMAND copy src\\ext\\oidn\\OpenImageDenoise_device_cuda.dll bin
        DEPENDS OpenImageDenoise_device_cuda
        )

add_custom_target(CP_USD ALL
        COMMAND
        copy ..\\src\\ext\\usd\\bin\\*.dll bin &&
        copy ..\\src\\ext\\usd\\lib\\*.dll bin &&
        xcopy ..\\src\\ext\\usd\\lib\\usd\\ bin\\usd\\  /s/y
        )