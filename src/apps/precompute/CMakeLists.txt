file(GLOB_RECURSE SOURCE_FILES *.c*)

vision_add_application(vision-precompute SOURCES ${SOURCE_FILES})