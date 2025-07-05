//
// Created by Zero on 2023/11/7.
//

#pragma once

#include "core/stl.h"
#include "dsl/dsl.h"
#include "core/parameter_set.h"
#include "base/scattering/interaction.h"
#include "base/encoded_object.h"
#include "base/mgr/pipeline.h"
#include "core/thread_pool.h"
#include "hotfix/hotfix.h"
#include "reservoir.h"

namespace vision {
using namespace ocarina;
struct SpatialResamplingParam {
public:
    float theta{};
    float depth_threshold{};
    float sampling_radius{};
    uint sample_num{};
    bool open{};

public:
    SpatialResamplingParam() = default;
    explicit SpatialResamplingParam(const ParameterSet &ps)
        : theta(ps["theta"].as_float(35)),
          depth_threshold(ps["depth"].as_float(0.3f)),
          sampling_radius(ps["radius"].as_float(30)),
          sample_num(ps["sample_num"].as_uint(1)),
          open{ps["open"].as_bool(true)} {
        sample_num = open ? sample_num : 1;
    }

    [[nodiscard]] float dot_threshold() const noexcept {
        return cosf(radians(theta));
    }
};

struct TemporalResamplingParam {
public:
    uint limit{};
    uint N{};
    float sampling_radius{};
    float theta{};
    float depth_threshold{};
    bool open{};
    bool mis{};

public:
    TemporalResamplingParam() = default;
    explicit TemporalResamplingParam(const ParameterSet &ps)
        : limit(ps["history_limit"].as_uint(5)),
          sampling_radius(ps["radius"].as_float(8)),
          theta(ps["theta"].as_float(35)),
          depth_threshold(ps["depth"].as_float(0.3)),
          open{ps["open"].as_bool(true)},
          N{ps["N"].as_uint(9)},
          mis{ps["mis"].as_bool(false)} {}

    [[nodiscard]] float dot_threshold() const noexcept {
        return cosf(radians(theta));
    }
};

}// namespace vision

namespace vision {
[[nodiscard]] inline Bool is_valid_neighbor(const SurfaceDataVar &cur_surface, const SurfaceDataVar &another_surface,
                                            const Float &dot_threshold, const Float &depth_threshold,
                                            const Float &diff_threshold) noexcept {
    Bool cond0 = abs_dot(cur_surface->normal(), another_surface->normal()) > dot_threshold;
    Bool cond1 = (abs(cur_surface->depth() - another_surface->depth()) / cur_surface->depth()) < depth_threshold;
    Bool cond2 = (abs(cur_surface->diffuse_factor() - another_surface->diffuse_factor())) / cur_surface->diffuse_factor() < diff_threshold;
    return cond0 && cond1 &&
           cond2 && cur_surface.hit->is_hit() && another_surface.hit->is_hit();
}

class ReSTIR : public EncodedObject, public Context, public RenderEnv, public GUI, public RuntimeObject {
protected:
    SpatialResamplingParam spatial_{};
    TemporalResamplingParam temporal_{};
    bool open_{true};
    uint max_age_{};
    float diff_factor_{0.3f};
    IlluminationIntegrator *integrator_{};

public:
    ReSTIR() = default;
    explicit ReSTIR(IlluminationIntegrator *integrator, const ParameterSet &desc)
        : integrator_(integrator),
          spatial_(desc["spatial"]),
          temporal_(desc["temporal"]),
          open_(desc["open"].as_bool(true)),
          diff_factor_(desc["diffuse_factor"].as_float(0.3f)),
          max_age_(desc["max_age"].as_uint(30)) {}
    VS_HOTFIX_MAKE_RESTORE(RuntimeObject, spatial_, temporal_, open_,
                           max_age_, diff_factor_, integrator_)
    OC_MAKE_MEMBER_SETTER(integrator)
    OC_MAKE_MEMBER_GETTER(open, )
    virtual void update_resolution(uint2 res) noexcept {}
    [[nodiscard]] Uint checkerboard_value() const noexcept {
        return frame_buffer().checkerboard_value(dispatch_idx().xy());
    }
    [[nodiscard]] auto prev_surfaces() const noexcept {
        return pipeline()->buffer_var<SurfaceData>(frame_buffer().prev_surfaces_index(frame_index()));
    }
    [[nodiscard]] auto cur_surfaces() const noexcept {
        return pipeline()->buffer_var<SurfaceData>(frame_buffer().cur_surfaces_index(frame_index()));
    }
    [[nodiscard]] auto prev_surface_extends() const noexcept {
        return pipeline()->buffer_var<SurfaceExtend>(frame_buffer().prev_surface_extends_index(frame_index()));
    }
    [[nodiscard]] auto cur_surface_extends() const noexcept {
        return pipeline()->buffer_var<SurfaceExtend>(frame_buffer().cur_surface_extends_index(frame_index()));
    }
    [[nodiscard]] Float3 cur_view_pos(const Bool &is_replace) const noexcept {
        Float3 view_pos;
        $if(is_replace) {
            view_pos = cur_surface_extends().read(dispatch_id()).view_pos;
        }
        $else {
            view_pos = scene().sensor()->device_position();
        };
        return view_pos;
    }
};
}// namespace vision
