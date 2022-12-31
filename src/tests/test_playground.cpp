//
// Created by Zero on 15/09/2022.
//

#include <math/box.h>
#include "core/stl.h"
#include "rhi/common.h"
#include "core/string_util.h"
#include "math/transform.h"
#include "math/geometry.h"
#include "math/constants.h"
#include "plugins/spectra/srgb2spec.h"

using namespace vision;
using namespace ocarina;

void fun(int *a) {
    cout << a << endl;
//    auto p = reinterpret_cast<int(*)[2][3]>(a);
//    auto v = *p;
//    cout << typeid(v).name();
cout << (*reinterpret_cast<int(*)[2][2][3]>(a))[1][1][1] << endl;
//cout << (reinterpret_cast<int(*)>(a))[2];
}

template<typename ...Args>
void set_dims(Args ...args) {
    vector<int> dims = {OC_FORWARD(args)...};
    int a;
}

template<typename T = std::byte ,int ...args>
class TB {
public:
};

int main(int argc, char *argv[]) {

//    auto tb = TB<int>();
//
//    int a[2][2][3] = {{{1, 2, 3}, {4, 5, 6}},
//                      {{-1,-2,-3}, {-4,-5,-6}}};
//
////    cout << typeid(decltype(&a)).name();
//
//    cout << a[1][1][1];
//
//
////    fun(reinterpret_cast<int*>(&a));
//
//    return 0;


    fs::path path(argv[0]);
    Context context(path.parent_path());
    Device device = context.create_device("cuda");
    Stream stream = device.create_stream();
//    context.clear_cache();
    float3 vec = make_float3(2);
    float4x4 mat = scale(2);
    Transform tsf = Transform(mat);

    auto ttt = Transform(scale(0.1f));

    auto m = rotation<H>(make_float3(1.f), 30.f, false);

    vec = tsf.apply_point(vec);

    int size = sizeof(sRGBToSpectrumTable_Data) / sizeof(float);

    auto buf = device.create_buffer<float,3,64,64,64,3>(size);
    auto buf2 = device.create_buffer<float>(size);

//    buf.set_dims(3,64,64,64,3);

buf.upload_immediately(sRGBToSpectrumTable_Data);
buf2.upload_immediately(sRGBToSpectrumTable_Data);

    auto arr = buf.dims.size();

    Kernel kernel = [&]() {
//        Var f3 = make_float3(1,2,1);
//        Frame frame = Frame(f3);
//        prints("{} {} {}",f3.xyz());
//        f3 = frame.to_local(f3);
//        prints("{} {} {}",f3.xyz());
        $for(i,0,10) {
            prints("{} {}", buf.read(i), buf2.read(i));
        };
    };
    auto shader = device.compile(kernel);

    stream << shader().dispatch(1) << synchronize() << commit();

    return 0;
}