add_subdirectory(films)
add_subdirectory(filters)
add_subdirectory(samplers)
add_subdirectory(lights)
add_subdirectory(lightsamplers)
add_subdirectory(integrators)
add_subdirectory(materials)
add_subdirectory(mediums)
add_subdirectory(sensors)
add_subdirectory(shadernodes)
add_subdirectory(shapes)
add_subdirectory(warpers)
add_subdirectory(spectra)
add_subdirectory(tonemappers)
add_subdirectory(denoisers)
add_subdirectory(uvspreaders)
add_subdirectory(pipelines)

add_library(vision-plugins INTERFACE)
target_link_libraries(vision-plugins INTERFACE
        vision-filter
        vision-shape
        vision-tonemapper
        vision-denoiser
        vision-uvspreader
        vision-lightsampler
        vision-sensor
        vision-pipeline
        vision-light
        vision-film
        vision-shadernode
        vision-integrator
        vision-warper
        vision-spectrum
        vision-material
        vision-medium
        vision-sampler)
