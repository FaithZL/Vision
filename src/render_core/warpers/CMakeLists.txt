add_library(vision-warper INTERFACE)
vision_add_plugin(alias CATEGORY warper SOURCES alias.cpp)
vision_add_plugin(alias2d CATEGORY warper SOURCES alias2d.cpp)