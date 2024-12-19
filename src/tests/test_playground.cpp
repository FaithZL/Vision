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
#include "core/util.h"

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

void foreach(int num, const std::function<void(int)> &f) {
    for (int i = 0; i < num; ++i) {
        f(i);
    }
}

template<typename T>
void TForeach(int num, const T &f) {
    for (int i = 0; i < num; ++i) {
        f(i);
    }
}

int main(int argc, char *argv[]) {
    Clock clk;

    float4x4 mat{float(argc)};
    int num = 100000000 * argc;
    double elps = 0;


    {
        mat = make_float4x4(argc);
        clk.start();
        TForeach(num, [&](int i) {
            mat = mat * mat;
        });
        clk.end();

        elps = clk.elapse_ms();
        cout << to_str(mat) << endl;
        cout << "lambda " << elps << "ms" << endl;
    }

    {
        clk.start();
        for (int i = 0; i < num; ++i) {
            mat = mat * mat;
        }
        clk.end();
        elps = clk.elapse_ms();
        cout << to_str(mat) << endl;
        cout << "naive " << elps << "ms" << endl;
    }

    {
        mat = make_float4x4(argc);
        clk.start();
        foreach (num, [&](int i) {
            mat = mat * mat;
        });
        clk.end();

        elps = clk.elapse_ms();
        cout << to_str(mat) << endl;
        cout << "std::function " << elps << "ms" << endl;
    }

    {
        mat = make_float4x4(argc);
        clk.start();
        TForeach(num, [&](int i) {
            mat = mat * mat;
        });
        clk.end();

        elps = clk.elapse_ms();
        cout << to_str(mat) << endl;
        cout << "lambda " << elps << "ms" << endl;
    }

    {
        clk.start();
        for (int i = 0; i < num; ++i) {
            mat = mat * mat;
        }
        clk.end();
        elps = clk.elapse_ms();
        cout << to_str(mat) << endl;
        cout << "naive " << elps << "ms" << endl;
    }

    {
        mat = make_float4x4(argc);
        clk.start();
        foreach (num, [&](int i) {
            mat = mat * mat;
        });
        clk.end();

        elps = clk.elapse_ms();
        cout << to_str(mat) << endl;
        cout << "std::function " << elps << "ms" << endl;
    }


    return 0;
}