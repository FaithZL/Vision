file(GLOB_RECURSE SOURCE_FILES *.c*)
vision_add_plugin(usd CATEGORY loader SOURCES ${SOURCE_FILES})