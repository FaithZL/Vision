add_library(vision-environment INTERFACE)
vision_add_plugin(spherical CATEGORY environment SOURCES spherical.cpp)
vision_add_plugin(directional CATEGORY environment SOURCES directional.cpp)
vision_add_plugin(black CATEGORY environment SOURCES black.cpp)