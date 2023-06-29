add_library(vision-pass INTERFACE)
vision_add_render_pass_plugin(accumulate CATEGORY pass SOURCES accumulate.cpp)
vision_add_render_pass_plugin(integrate CATEGORY pass SOURCES integrate.cpp)
vision_add_render_pass_plugin(tonemapping CATEGORY pass SOURCES tonemapping.cpp)