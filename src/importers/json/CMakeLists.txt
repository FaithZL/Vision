file(GLOB_RECURSE SOURCE_FILES *.c*)
vision_add_plugin(json CATEGORY importer SOURCES ${SOURCE_FILES})