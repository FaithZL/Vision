add_library(vision-pipeline INTERFACE)

file(GLOB_RECURSE BAKE_SOURCE_FILES bake/*.c*)
vision_add_plugin(baker CATEGORY pipeline SOURCES ${BAKE_SOURCE_FILES})

vision_add_plugin(offline CATEGORY pipeline SOURCES offline.cpp)
vision_add_plugin(real_time CATEGORY pipeline SOURCES real_time.cpp)
