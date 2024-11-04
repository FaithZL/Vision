file(GLOB_RECURSE SOURCE_FILES *.c*)

vision_add_application(vision-gui SOURCES ${SOURCE_FILES})

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    add_custom_target(export_to_python ALL
            DEPENDS vision-gui ocapi
            COMMAND ${CMAKE_COMMAND} -E echo "export libraries after all targets are built..."
            COMMAND ${CMAKE_COMMAND} -E echo "start copy"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND python.exe ${CMAKE_SOURCE_DIR}/src/scripts/after_build_pyd.py ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}
            COMMAND ${CMAKE_COMMAND} -E echo "Complete copy!"
            COMMENT "Executing script after building all targets"
    )
else ()
    add_custom_target(export_to_python ALL
            DEPENDS vision-gui ocapi
            COMMAND ${CMAKE_COMMAND} -E echo "export libraries after all targets are built..."
            COMMAND ${CMAKE_COMMAND} -E echo "start copy"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND python.exe ${CMAKE_SOURCE_DIR}/src/scripts/after_build_pyd.py ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E echo "Complete copy!"
            COMMENT "Executing script after building all targets"
    )
endif ()