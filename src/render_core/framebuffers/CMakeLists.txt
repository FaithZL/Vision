add_library(vision-framebuffer INTERFACE)
vision_add_plugin(raster CATEGORY framebuffer SOURCES raster.cpp)
vision_add_plugin(rt CATEGORY framebuffer SOURCES rt.cpp)