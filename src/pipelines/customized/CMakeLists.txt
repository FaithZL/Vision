file(GLOB_RECURSE SOURCE_FILES *.c*)
vision_add_plugin(customized
        CATEGORY pipeline
        DEPENDENCE "vision-render_graph"
        SOURCES ${SOURCE_FILES})
