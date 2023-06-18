add_library(vision-pipeline INTERFACE)

add_subdirectory(baker)

vision_add_plugin(offline CATEGORY pipeline SOURCES offline.cpp)
vision_add_plugin(real_time CATEGORY pipeline SOURCES real_time.cpp)
