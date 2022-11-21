add_library(vision-warpers INTERFACE)
vision_add_plugin(inverse_trans CATEGORY warper SOURCES inverse_trans.cpp)
vision_add_plugin(alias_table CATEGORY warper SOURCES alias_table.cpp)