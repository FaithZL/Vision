add_library(vision-rasterizer INTERFACE)
vision_add_plugin(software CATEGORY rasterizer SOURCES software.cpp)
vision_add_plugin(gl CATEGORY rasterizer SOURCES gl.cpp)