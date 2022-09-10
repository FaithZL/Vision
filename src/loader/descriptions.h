//
// Created by Zero on 10/09/2022.
//

#pragma once

#include "core/stl.h"

namespace vision {

using namespace ocarina;

class Description {
protected:
    std::string _type;

public:
    std::string name;
    void set_type(const std::string &type) { _type = type; }
    OC_NODISCARD const std::string &type() const { return _type; }
};

}// namespace vision