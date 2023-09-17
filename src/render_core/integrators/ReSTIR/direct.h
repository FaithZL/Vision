//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "util.h"
#include "base/serial_object.h"
#include "base/mgr/global.h"

namespace vision {
/**
 * generate initial candidates
 * evaluate visibility for initial candidates
 * temporal reuse
 * spatial reuse and iterate
 */
class ReSTIRDI : public SerialObject, public Ctx {
private:
    uint M{};
    uint n{};
    Buffer<Reservoir> _reservoirs;
    Buffer<Reservoir> _prev_reservoirs;

    /**
     * generate initial candidates
     * check visibility
     * temporal reuse
     */
    Shader<void(uint)> _shader0;
    /**
     * spatial reuse and shading
     */
    Shader<void(uint)> _shader1;

public:
    explicit ReSTIRDI(uint M) : M(M) {}
    void prepare() noexcept;
    void compile() noexcept {
        compile_shader0();
        compile_shader1();
    }
    [[nodiscard]] OCReservoir RIS(Uint2 pixel, Bool hit, const Interaction &it, SampledWavelengths &swl) const noexcept;
    void compile_shader0() noexcept;
    void compile_shader1() noexcept;
    [[nodiscard]] CommandList estimate() const noexcept;
};

}// namespace vision