add_library(vision-rasterizer INTERFACE)
vision_add_plugin(software CATEGORY sampler SOURCES software.cpp)
vision_add_plugin(gl CATEGORY sampler SOURCES gl.cpp)
