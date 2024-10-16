add_library(vision-uvunwrapper INTERFACE)

vision_add_plugin(xatlas CATEGORY uvunwrapper DEPENDENCE "vision-ext-xatlas" SOURCES xatlas.cpp)