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
     * Serialize the data to managed memory
     * for upload to device memory
     */
    virtual void encode_data() noexcept {
        encode(_datas);
    }

    /**
     * encode data, initialize device buffer and register buffer to resource array
     */
    virtual void prepare_data() noexcept {
        encode_data();
        datas().reset_device_buffer_immediately(Global::instance().device());
        datas().register_self();
    }

    /**
     * update data to managed memory
     * tips: Called on the host side code
     */
    virtual void update_data() noexcept {
        update(_datas);
    }

    /**
     * load data from device memory
     * tips: Called on the device side code
     */
    virtual void load_data() noexcept {
        DataAccessor<float> da = {0, _datas};
        decode(&da);
    }

    virtual void upload_immediately() noexcept {
        if (datas().host_buffer().empty()) {
            return;
        }
        datas().upload_immediately();
    }

    [[nodiscard]] virtual BufferUploadCommand *upload_sync() noexcept {
        return datas().upload_sync();
    }

    [[nodiscard]] virtual BufferUploadCommand *upload() noexcept {
        return datas().upload();
    }
};

}// namespace vision