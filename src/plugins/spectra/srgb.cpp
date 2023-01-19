//
// Created by Zero on 21/12/2022.
//

#include "base/color/spectrum.h"

namespace vision {

class SRGBSpectrum : public Spectrum {
public:
    explicit SRGBSpectrum(const SpectrumDesc &desc)
        : Spectrum(desc) {}
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SRGBSpectrum)