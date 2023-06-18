file(GLOB_RECURSE SOURCE_FILES *.c*)
vision_add_plugin(baker CATEGORY pipeline SOURCES ${SOURCE_FILES})