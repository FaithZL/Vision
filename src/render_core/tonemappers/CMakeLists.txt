add_library(vision-tonemapper INTERFACE)
vision_add_plugin(linear CATEGORY tonemapper SOURCES linear.cpp)
vision_add_plugin(reinhard CATEGORY tonemapper SOURCES reinhard.cpp)
vision_add_plugin(aces CATEGORY tonemapper SOURCES aces.cpp)
