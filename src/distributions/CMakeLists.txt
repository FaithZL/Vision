add_library(vision-distributions INTERFACE)
vision_add_plugin(dichotomy CATEGORY distribution SOURCES dichotomy.cpp)
vision_add_plugin(alias_table CATEGORY distribution SOURCES alias_table.cpp)