add_library(vision-lightsampler INTERFACE)
vision_add_plugin(uniform CATEGORY lightsampler SOURCES uniform.cpp)
vision_add_plugin(power CATEGORY lightsampler SOURCES power.cpp)
vision_add_plugin(bvh CATEGORY lightsampler SOURCES bvh.cpp)