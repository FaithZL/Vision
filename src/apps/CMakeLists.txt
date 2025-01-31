file(GLOB_RECURSE SOURCE_FILES *.c*)

vision_add_application(vision-gui SOURCES application.cpp vision-gui.cpp)
vision_add_application(vision-precompute_albedo SOURCES precompute_albedo.cpp)
