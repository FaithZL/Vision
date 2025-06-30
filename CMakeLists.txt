
cmake_minimum_required(VERSION 3.22)
set(CMAKE_POLICY_VERSION_MINIMUM "3.22")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include_directories(src)

set(PYTHON_PATH  "DEFAULT_VALUE_IF_NOTHING_ELSE_SET")
set(CUSTOM_PYTHON_PATH "" CACHE PATH "Optional: Specify a custom path to the Python executable if not found automatically.")

find_program(PYTHON_EXECUTABLE NAMES python python3 python2
                             PATHS ENV PATH)

if (PYTHON_EXECUTABLE)
    message(STATUS "Found Python executable: ${PYTHON_EXECUTABLE}")
    set(PYTHON_PATH "${PYTHON_EXECUTABLE}")
else()
    message(STATUS "Python executable not found in standard paths.")
    if (CUSTOM_PYTHON_PATH) # Check if the user provided a custom path
        message(STATUS "Using custom Python path from argument: ${CUSTOM_PYTHON_PATH}")
        set(PYTHON_PATH "${CUSTOM_PYTHON_PATH}")
    else()
        message(FATAL_ERROR "No custom Python path provided.")
    endif()
endif()


project(Vision)

add_definitions(-D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/")

set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(OCARINA_PY "Build python export" FALSE)
option(OIDN_ON "Build oidn" TRUE)

#set(OCARINA_BUILD_TESTS FALSE)
#set(OCARINA_PY TRUE)
#set(OIDN_ON FALSE)

if (MSVC)
add_compile_options(
        /wd4996
)
endif ()

function(vision_add_application name)
    cmake_parse_arguments(APP "" "" "SOURCES" ${ARGN})
    add_executable(${name} ${APP_SOURCES})
    target_compile_features(${name} PRIVATE c_std_11 cxx_std_20)
    target_link_libraries(${name} PRIVATE vision-all_libs ${STDCXX_EXTRA_LIBS})
    set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
endfunction()

message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")

message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")

add_subdirectory(src)

if (${OIDN_ON})
    add_custom_target(CP_OIDN_CUDA ALL
            COMMAND copy src\\ext\\oidn\\OpenImageDenoise_device_cuda.dll bin
            DEPENDS OpenImageDenoise_device_cuda
    )
endif ()
