add_library(vision-materials INTERFACE)
vision_add_plugin(matte CATEGORY material SOURCES matte.cpp)
vision_add_plugin(null CATEGORY material SOURCES null.cpp)
vision_add_plugin(glass CATEGORY material SOURCES glass.cpp)