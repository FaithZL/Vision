add_library(vision-textures INTERFACE)
vision_add_plugin(constant CATEGORY texture SOURCES constant.cpp)
vision_add_plugin(image CATEGORY texture SOURCES image.cpp)