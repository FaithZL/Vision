# Vision
Vision is a GPU physically based renderer,
The vision renderer uses embeded DSL codegen technology derived from luisa compute https://github.com/LuisaGroup/LuisaCompute, which is a nice invention! 

### Render algorithm
| Feature                                                 | Progress  |
|---------------------------------------------------------|-----------|
| Megakernel path tracing                                 | Done      |
| Wavefront path tracing                                  | Planned   |
| Bidirectional path tracing                              | Planned   |
| Photon mapping                                          | Planned   |

### Reconstruction Filters
| Feature                      | Progress    |
|------------------------------|-------------|
| Filter Importance Sampling   | Planned     |
| Mitchell-Netravali Filter    | Planned     |
| Box Filter                   | Done        |
| Triangle Filter              | Planned     |
| Gaussian Filter              | Planned     |
| Lanczos Windowed Sinc Filter | Planned     |

### Materials
| Feature                      | Progress    |
|------------------------------|-------------|
| metal                        | Done        |
| matte                        | Done        |
| glass                        | Done        |
| mirror                       | Done        |
| substrate                    | Done        |
| disney                       | Done        |
| hair                         | Planned     |
| BSSRDF                       | Planned     |

### Sensor
| Feature                                   | Progress    |
|-------------------------------------------|-------------|
| Pinhole Cameras                           | Done        |
| Thin-Lens Cameras                         | Planned     |
| Realistic Cameras                         | Planned     |
| Fish-Eye Cameras                          | Planned     |
| Geometry surface(for light map baker)     | Planned     |

### Illumination
| Feature                                       | Progress    |
|-----------------------------------------------|-------------|
| Area Lights, emission                         |  done       |
| projector                                     |  done       |
| spotlight                                     |  done       |
| pointlight                                    |  done       |
| IES                                           |  Planned    |
| HDRI Environment Maps                         |  done       |
| Uniform-Distribution Light Selection Strategy |  done       |
| Power-Distribution Light Selection Strategy   |  Planned    |
| BVH Light Selection Strategy                  |  Planned    |

### Backends
| Feature             | Progress                                            |
|---------------------|-----------------------------------------------------|
| GPU                 | Done (with CUDA + OptiX)                            |

### Exporters
| Feature             | Progress                                            |
|---------------------|-----------------------------------------------------|
| Blender             | Planned                                             |
| 3DS Max             | Planned                                             |

Part of the scene comes from https://benedikt-bitterli.me/resources/

![](gallery/staircase.png)
![](gallery/bathroom.png)
![](gallery/classroom-1024spp.png)
![](gallery/classroom-fog-1024spp.png)
![](gallery/output.png)
![](gallery/glass-of-water-1024spp.png)
![](gallery/spaceship-1024spp.png)
![](gallery/kitchen0.png)
![](gallery/cornell-box-fog.png)
![](gallery/projector.png)
![](gallery/tv.png)
![](gallery/cornell-box-fog-projector.png)
![](gallery/cbox-sss.png)
![](gallery/cbox-dragon.png)
