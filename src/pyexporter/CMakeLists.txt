file(GLOB_RECURSE HEADER_FILES *.h*)
file(GLOB_RECURSE SOURCE_FILES *.c*)

pybind11_add_module(vsapi ${SOURCE_FILES})
target_link_libraries(vsapi PUBLIC
        vision-all_libs)


if (CMAKE_GENERATOR MATCHES "Visual Studio")
    add_custom_target(export_to_python ALL
            DEPENDS vsapi ocapi
            COMMAND ${CMAKE_COMMAND} -E echo "export libraries after all targets are built..."
            COMMAND ${CMAKE_COMMAND} -E echo "start copy"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${PYTHON_PATH} ${CMAKE_SOURCE_DIR}/src/scripts/after_build_pyd.py ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}
            COMMAND ${CMAKE_COMMAND} -E echo "Complete copy!"
            COMMENT "Executing script after building all targets"
    )
else ()
    add_custom_target(export_to_python ALL
            DEPENDS vsapi ocapi
            COMMAND ${CMAKE_COMMAND} -E echo "export libraries after all targets are built..."
            COMMAND ${CMAKE_COMMAND} -E echo "start copy"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${PYTHON_PATH} ${CMAKE_SOURCE_DIR}/src/scripts/after_build_pyd.py ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E echo "Complete copy!"
            COMMENT "Executing script after building all targets"
    )
endif ()