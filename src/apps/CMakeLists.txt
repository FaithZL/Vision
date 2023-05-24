file(GLOB_RECURSE SOURCE_FILES *.c*)

vision_add_dll(vision SOURCES ${SOURCE_FILES})
vision_add_application(vision-gui SOURCES ${SOURCE_FILES})
