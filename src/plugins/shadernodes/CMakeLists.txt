add_library(vision-shadernode INTERFACE)
vision_add_plugin(constant CATEGORY shadernode SOURCES constant.cpp)
vision_add_plugin(image CATEGORY shadernode SOURCES image.cpp)
vision_add_plugin(lerp CATEGORY shadernode SOURCES lerp.cpp)