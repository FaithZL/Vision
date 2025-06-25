add_library(vision-ray_generator INTERFACE)
vision_add_plugin(normal CATEGORY ray_generator SOURCES normal.cpp)
vision_add_plugin(lightfield CATEGORY ray_generator SOURCES lightfield.cpp)