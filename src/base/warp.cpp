//
// Created by zhu on 2023/5/3.
//

#include "warper.h"

namespace vision {

void Warper2D::build(vector<float> weights, uint2 res) noexcept {
}
Float Warper2D::func_at(Uint2 coord) const noexcept {
    return ocarina::Float();
}
Float Warper2D::PDF(Float2 p) const noexcept {
    return ocarina::Float();
}
float Warper2D::integral() const noexcept {
    return 0;
}
Float2 Warper2D::sample_continuous(Float2 u, Float *pdf, Uint2 *coord) const noexcept {
    return ocarina::Float2();
}

}// namespace vision