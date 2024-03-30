//
// Created by Zero on 2023/8/21.
//

#include "base/illumination/ies.h"
#include "core/string_util.h"
#include "math/transform.h"

using namespace ocarina;

int main() {

    auto path = "E:\\work\\renderer\\Vision\\res\\ies\\6.ies";

    auto m = vision::rotation_x<H>(80);
    auto q = vision::quaternion::from_float3x3(make_float3x3(m));


    auto m2 = q.to_float3x3();
    auto a = q.axis();
    auto t = degrees(q.theta());

    auto q2 = vision::quaternion ::from_float3x3(m2);

    auto a2 = q2.axis();
    auto t2 = degrees(q2.theta());

    string ies = from_file(path);
    vision::IESFile ies_file;

    ies_file.load(ies);

    return 0;
}