//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "util.h"
#include "base/serial_object.h"
#include "base/mgr/global.h"

namespace vision {

class ReSTIRDI : public SerialObject, public Ctx {
private:
    uint M{};

public:
    explicit ReSTIRDI(uint M) : M(M) {}
};

}// namespace vision