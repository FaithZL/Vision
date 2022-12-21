add_library(vision-spectrum INTERFACE)
vision_add_plugin(hero CATEGORY spectrum SOURCES hero.cpp)
vision_add_plugin(srgb CATEGORY spectrum SOURCES srgb.cpp)