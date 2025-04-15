//
// Created by Zero on 2025/4/15.
//

#pragma once

#include "base/scattering/medium.h"
#include "UI/polymorphic.h"

namespace vision {

class MediumRegistry : public GUI, public Observer {
private:
    OC_MAKE_INSTANCE_CONSTRUCTOR(MediumRegistry, s_medium_registry)

private:
    PolymorphicGUI<SP<Medium>> mediums_;

public:
    OC_MAKE_INSTANCE_FUNC_DECL(MediumRegistry)
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    VS_MAKE_GUI_STATUS_FUNC(GUI, mediums_)
};

}// namespace vision