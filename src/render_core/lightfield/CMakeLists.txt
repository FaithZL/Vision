add_library(vision-lightfield INTERFACE)
vision_add_plugin(encode CATEGORY lightfield SOURCES encode.cpp)
vision_add_plugin(perspective CATEGORY lightfield SOURCES perspective.cpp)
