file(GLOB_RECURSE SOURCE_FILES *.c*)
vision_add_plugin(bake CATEGORY pipeline SOURCES ${SOURCE_FILES})