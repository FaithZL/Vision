add_library(vision-integrator INTERFACE)

add_subdirectory(ReSTIR)
add_subdirectory(surfel)
add_subdirectory(adaptive)
add_subdirectory(lightfield)

vision_add_plugin(pt CATEGORY integrator DEPENDENCE "vision-integrator-adaptive" SOURCES pt.cpp)
vision_add_plugin(lfpt CATEGORY integrator DEPENDENCE "vision-integrator-lightfield" SOURCES lfpt.cpp)
vision_add_plugin(rt CATEGORY integrator DEPENDENCE "vision-integrator-surfel;vision-integrator-ReSTIR" SOURCES rt.cpp)
vision_add_plugin(ao CATEGORY integrator SOURCES ao.cpp)

