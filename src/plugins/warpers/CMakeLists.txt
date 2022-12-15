add_library(vision-warpers INTERFACE)
vision_add_plugin(inverse_trans CATEGORY warper SOURCES inverse_trans.cpp)
vision_add_plugin(alias CATEGORY warper SOURCES alias.cpp)
vision_add_plugin(alias2d CATEGORY warper SOURCES alias2d.cpp)