add_library(vision-spectrums INTERFACE)
vision_add_plugin(constant CATEGORY spectrum SOURCES hero.cpp)
vision_add_plugin(image CATEGORY spectrum SOURCES srgb.cpp)