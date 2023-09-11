file(GLOB_RECURSE SOURCE_FILES *.c*)
vision_add_render_pass_plugin(customized CATEGORY pipeline SOURCES ${SOURCE_FILES})