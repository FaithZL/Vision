add_library(vision-denoiser INTERFACE)

if (${OIDN_ON})
    vision_add_plugin(oidn CATEGORY denoiser DEPENDENCE "vision-ext-oidn" SOURCES oidn.cpp)
endif ()


vision_add_plugin(optix CATEGORY denoiser SOURCES optix.cpp)
add_subdirectory(SVGF)