//
// Created by Zero on 2023/8/21.
//

#include "base/illumination/ies.h"
#include "core/string_util.h"
#include "math/transform.h"

using namespace ocarina;
using namespace vision;
int main() {

    auto path = "E:\\work\\renderer\\Vision\\res\\ies\\6.ies";


    auto T =  transform::translation(1.f,2.f,3.f);
    auto R = transform::rotation<H>(make_float3(1,1,1), 45);
    auto S = transform::scale(0.5f);

    auto M = T * R * S;

    float3 t;
    quaternion r;
    float3 s;

    decompose(M, &t, &r, &s);

    float3 axis = r.axis();
    float deg = degrees(r.theta());

    string ies = from_file(path);
    vision::IESFile ies_file;

    ies_file.load(ies);

    return 0;
}