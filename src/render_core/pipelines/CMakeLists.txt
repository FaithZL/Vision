add_library(vision-pipeline INTERFACE)
vision_add_plugin(baker CATEGORY pipeline SOURCES baker.cpp)
vision_add_plugin(render CATEGORY pipeline SOURCES render.cpp)
