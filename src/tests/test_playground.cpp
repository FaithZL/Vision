//
// Created by Zero on 15/09/2022.
//

#include "math/box.h"
#include "core/stl.h"
#include "rhi/common.h"
#include "core/string_util.h"
#include "math/transform.h"
#include "base/scattering/interaction.h"
#include "math/geometry.h"
#include "math/constants.h"
#include "render_core/spectra/srgb2spec.h"
#include "base/color/spd.h"

using namespace vision;
using namespace ocarina;

float eta_bk7(float lambda) {
    float f = 1.03961212 * sqr(lambda) / (sqr(lambda) - 0.00600069867) +
              0.231792344 * sqr(lambda) / (sqr(lambda) - 0.0200179144) +
              1.01046945 * sqr(lambda) / (sqr(lambda) - 103.560653);
    return sqrt(f + 1);
}

float eta_LASF9(float lambda) {
    auto f = 2.00029547f * sqr(lambda) / (sqr(lambda) - 0.0121426017f) +
             0.298926886f * sqr(lambda) / (sqr(lambda) - 0.0538736236f) +
             1.80691843f * sqr(lambda) / (sqr(lambda) - 156.530829f);
    return sqrt(f + 1);

}

struct NODE {

};

struct Good : public std::enable_shared_from_this<Good> , public NODE{

};

struct Better : public Good {};

void test() {
    auto p = ocarina::new_with_allocator<Better>();
    shared_ptr<NODE> good = shared_ptr<NODE>(p);

//    auto g1 = good->shared_from_this();

    good.reset();

    int i =0;
}

int main(int argc, char *argv[]) {

    test();
    return 0;

    uint type_id = 0u;
    uint inst_id = 0u;
    vector<uint> func{1, 2, 5, 6, 3};
    uint accum = 0u;
    uint index = 13;
    for (int i = 0; i < func.size(); ++i) {
        type_id = select(index >= accum, i, type_id);
        inst_id = select(index >= accum, index - accum, inst_id);
        accum += func[i];
    }

    cout << "index = " << index << endl;
    cout << "type_id = " << type_id << endl;
    cout << "inst_id = " << inst_id << endl;

    uint id = 0;
    for (int i = 0; i < type_id; ++i) {
        id += func[i];
    }
    index = id + inst_id;
    cout << "index = " << index << endl;

    auto tmp = encode_id<H>(1237, 113);



    //    auto [inst, type] = decode_id<H>(tmp);
    //    cout << tmp << endl;
    //    cout << inst << endl << type << endl;
    //    cout << std::hex << InvalidUI32;
    //    float e = eta_LASF9(0.83);
    //    float e2 = eta_LASF9(0.36);
    return 0;
}