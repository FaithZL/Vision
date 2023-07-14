file(GLOB_RECURSE SOURCE_FILES *.c*)
vision_add_plugin(assimp CATEGORY loader SOURCES ${SOURCE_FILES})