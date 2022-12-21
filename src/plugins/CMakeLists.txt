add_subdirectory(films)
add_subdirectory(filters)
add_subdirectory(samplers)
add_subdirectory(lights)
add_subdirectory(lightsamplers)
add_subdirectory(integrators)
add_subdirectory(materials)
add_subdirectory(mediums)
add_subdirectory(sensors)
add_subdirectory(textures)
add_subdirectory(shapes)
add_subdirectory(warpers)
add_subdirectory(spectra)

add_library(vision-plugins INTERFACE)
target_link_libraries(vision-plugins INTERFACE
        vision-filter
        vision-shape
        vision-lightsampler
        vision-sensor
        vision-light
        vision-film
        vision-texture
        vision-integrator
        vision-warper
        vision-spectrum
        vision-material
        vision-medium
        vision-sampler)
