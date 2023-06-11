//
// Created by Zero on 2023/5/30.
//

#include "base/denoiser.h"
#include "OpenImageDenoise/oidn.hpp"

namespace vision {

using namespace ocarina;

class OidnDenoiser : public Denoiser {
private:
    oidn::DeviceRef _device{};

public:
    explicit OidnDenoiser(const DenoiserDesc &desc)
        : Denoiser(desc),
          _device{oidn::DeviceRef(oidn::newDevice(oidn::DeviceType::CUDA))} {
        _device.commit();
    }

    [[nodiscard]] oidn::FilterRef create_filter() const noexcept {
        switch (_mode) {
            case RT:
                return _device.newFilter("RT");
            case RTLightmap:
                return _device.newFilter("RTLightmap");
            default:
                break;
        }
        return nullptr;
    }

    void apply(uint2 res, float4 *output, float4 *color,
               float4 *normal, float4 *albedo) noexcept override {
        oidn::FilterRef filter = create_filter();
        filter.setImage("output", output, oidn::Format::Float3, res.x, res.y, 0, sizeof(float4));
        filter.setImage("color", color, oidn::Format::Float3, res.x, res.y, 0, sizeof(float4));
        if (normal && albedo) {
            filter.setImage("normal", normal, oidn::Format::Float3, res.x, res.y, 0, sizeof(float4));
            filter.setImage("albedo", albedo, oidn::Format::Float3, res.x, res.y, 0, sizeof(float4));
        }
        // color image is HDR
        filter.set("hdr", true);
        filter.commit();
        filter.execute();

        const char *errorMessage;
        if (_device.getError(errorMessage) != oidn::Error::None) {
            OC_ERROR_FORMAT("oidn error: {}", errorMessage)
        }
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::OidnDenoiser)