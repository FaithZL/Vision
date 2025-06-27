add_library(vision-raygenerator INTERFACE)
vision_add_plugin(normal CATEGORY raygenerator SOURCES normal.cpp)
vision_add_plugin(lightfield CATEGORY raygenerator SOURCES lightfield.cpp)