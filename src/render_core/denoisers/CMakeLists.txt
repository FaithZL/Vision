add_library(vision-denoiser INTERFACE)
vision_add_plugin(oidn CATEGORY denoiser SOURCES oidn.cpp)
vision_add_plugin(optix CATEGORY denoiser SOURCES optix.cpp)