file(GLOB_RECURSE BAKE_SOURCE_FILES *.c*)
vision_add_plugin(baker CATEGORY pipeline SOURCES ${BAKE_SOURCE_FILES})