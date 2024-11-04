file(GLOB_RECURSE HEADER_FILES *.h*)
file(GLOB_RECURSE SOURCE_FILES *.c*)

add_library(vsapi SHARED ${SOURCE_FILES})
target_link_libraries(vsapi PUBLIC
        vision-all_libs)

set_target_properties(vsapi PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        WINDOWS_EXPORT_ALL_SYMBOLS ON)


if (CMAKE_GENERATOR MATCHES "Visual Studio")
    add_custom_target(export_to_python ALL
            DEPENDS vsapi ocapi
            COMMAND ${CMAKE_COMMAND} -E echo "export libraries after all targets are built..."
            COMMAND ${CMAKE_COMMAND} -E echo "start copy"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND python.exe ${CMAKE_SOURCE_DIR}/src/scripts/after_build_pyd.py ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}
            COMMAND ${CMAKE_COMMAND} -E echo "Complete copy!"
            COMMENT "Executing script after building all targets"
    )
else ()
    add_custom_target(export_to_python ALL
            DEPENDS vsapi ocapi
            COMMAND ${CMAKE_COMMAND} -E echo "export libraries after all targets are built..."
            COMMAND ${CMAKE_COMMAND} -E echo "start copy"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND python.exe ${CMAKE_SOURCE_DIR}/src/scripts/after_build_pyd.py ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E echo "Complete copy!"
            COMMENT "Executing script after building all targets"
    )
endif ()