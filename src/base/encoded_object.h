//
// Created by Zero on 2023/6/18.
//

#pragma once

#include "core/stl.h"
#include "dsl/encodable.h"
#include "node.h"
#include "mgr/global.h"

namespace vision {

using namespace ocarina;

/**
 * Provides serialization interface for integrators,
 * cameras and other monolithic objects
 * that do not need to be added to polymorphic lists
 */
class EncodedObject : public Encodable<buffer_ty> {
protected:
    RegistrableManaged<buffer_ty> datas_{Global::instance().bindless_array()};

protected:
    EncodedObject() = default;

public:
    [[nodiscard]] RegistrableManaged<buffer_ty> &datas() noexcept { return datas_; }
    [[nodiscard]] const RegistrableManaged<buffer_ty> &datas() const noexcept { return datas_; }

    /**
     * Serialize the data to managed memory
     * for upload to device memory
     */
    virtual void encode_data() noexcept {
        encode(datas_);
    }

    /**
     * encode data, initialize device buffer and register buffer to resource array
     */
    virtual void prepare_data() noexcept {
        encode_data();
        datas().reset_device_buffer_immediately(Global::instance().device(),
                                                ocarina::format("EncodedObject: {}::data_", typeid(*this).name()));
        datas().register_self();
    }

    /**
     * update data to managed memory
     * tips: Called on the host side code
     */
    virtual void update_data() noexcept {
        update(datas_);
    }

    /**
     * load data from device memory
     * tips: Called on the device side code
     */
    virtual void load_data() noexcept {
        DataAccessor<buffer_ty> da = {0, datas_};
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