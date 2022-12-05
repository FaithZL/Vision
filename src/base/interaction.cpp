//
// Created by Zero on 05/12/2022.
//

#include "interaction.h"
#include "medium.h"

namespace vision {
using namespace ocarina;
Interaction::Interaction() {
    phase = make_shared<HenyeyGreenstein>();
    mi = make_shared<MediumInterface>();
}

RayState Interaction::spawn_ray_state(const Float3 &dir) const noexcept {
    OCRay ray = vision::spawn_ray(pos, g_uvn.normal(), dir);
    Uchar medium = mi ? select(dot(g_uvn.normal(), dir) > 0, mi->outside, mi->inside) : InvalidUI8;
    return {.ray = ray, .ior = 1.f, .medium = medium};
}

void Interaction::set_medium(const Uchar &inside, const Uchar &outside) {
    if (mi) {
        mi->inside = inside;
        mi->outside = outside;
    }
}

}// namespace vision