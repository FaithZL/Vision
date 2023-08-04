add_library(vision-rasterizer INTERFACE)
vision_add_plugin(gl CATEGORY rasterizer SOURCES gl.cpp)
vision_add_plugin(cpu CATEGORY rasterizer SOURCES cpu.cpp)
