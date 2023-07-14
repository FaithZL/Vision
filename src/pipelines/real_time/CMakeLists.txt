file(GLOB_RECURSE SOURCE_FILES *.c*)
vision_add_render_pass_plugin(real_time CATEGORY pipeline SOURCES ${SOURCE_FILES})