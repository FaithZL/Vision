//
// Created by Zero on 2023/6/18.
//

#pragma once

#include "core/stl.h"
#include "dsl/serialize.h"
#include "node.h"
#include "mgr/global.h"

namespace vision {

using namespace ocarina;

/**
 * Provides serialization interface for integrators,
 * cameras and other monolithic objects
 * that do not need to be added to polymorphic lists
 */
class SerialObject : public Serializable<float> {
protected:
    RegistrableManaged<float> _datas{Global::instance().resource_array()};

protected:
    SerialObject() = default;

public:
    [[nodiscard]] RegistrableManaged<float> &datas() noexcept { return _datas; }
    [[nodiscard]] const RegistrableManaged<float> &datas() const noexcept { return _datas; }

    /**
     * Serialize the data for upload to device memory
     */
    virtual void encode_data() noexcept {
        encode(_datas);
    }

    /**
     * load data from device memory
     */
    virtual void load_data() noexcept {
        DataAccessor<float> da = {0, _datas};
        decode(&da);
    }
};

}// namespace vision