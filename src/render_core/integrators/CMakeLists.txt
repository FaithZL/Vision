add_library(vision-integrator INTERFACE)
vision_add_plugin(pt CATEGORY integrator SOURCES pt.cpp)
vision_add_plugin(ao CATEGORY integrator SOURCES ao.cpp)
