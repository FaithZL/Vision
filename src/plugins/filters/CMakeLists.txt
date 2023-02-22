add_library(vision-filter INTERFACE)
vision_add_plugin(box CATEGORY filter SOURCES box.cpp)
vision_add_plugin(triangle CATEGORY filter SOURCES triangle.cpp)