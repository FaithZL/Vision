//
// Created by ling.zhu on 2025/1/30.
//

#include "ltc_sheen.h"
#include "base/mgr/pipeline.h"

namespace vision {

SheenLTCTable &SheenLTCTable::instance() {
    if (s_sheen_table == nullptr) {
        s_sheen_table = new SheenLTCTable();
    }
    return *s_sheen_table;
}

void SheenLTCTable::destroy_instance() {
    if (s_sheen_table) {
        delete s_sheen_table;
        s_sheen_table = nullptr;
    }
}

void SheenLTCTable::init() noexcept {
    if (approx_.handle()) {
        return;
    }
    
}

Float4 SheenLTCTable::sample_approx(const Float &alpha, const Float &cos_theta) noexcept {
    return approx_.sample(4, make_float2(alpha, cos_theta)).as_vec4();
}

Float4 SheenLTCTable::sample_volume(const Float &alpha, const Float &cos_theta) noexcept {
    return volume_.sample(4, make_float2(alpha, cos_theta)).as_vec4();
}

}// namespace vision