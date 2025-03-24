//
// Created by Zero on 2025/3/23.
//

#include "encoded_object.h"

namespace vision {

void EncodedObject::encode_data() noexcept {
    auto size = aligned_size();
    datas_.host_buffer().resize(size / sizeof(buffer_ty));
    encode(datas_);
}

void EncodedObject::prepare_data() noexcept {
    encode_data();
    datas().reset_device_buffer_immediately(Global::instance().device(),
                                            ocarina::format("EncodedObject: {}::data_", typeid(*this).name()));
    datas().register_self();
}

void EncodedObject::update_data() noexcept {
    encode(datas_);
}

void EncodedObject::load_data() noexcept {
    DataAccessor da = {0, datas_};
    DynamicArray<buffer_ty> array = da.load_dynamic_array<buffer_ty>(aligned_size() / 4);
    decode(array);
}

void EncodedObject::upload_immediately() noexcept {
    if (datas().host_buffer().empty()) {
        return;
    }
    datas().upload_immediately();
}

BufferUploadCommand *EncodedObject::upload_sync() noexcept {
    return datas().upload_sync();
}

BufferUploadCommand *EncodedObject::upload() noexcept {
    return datas().upload();
}

}// namespace vision