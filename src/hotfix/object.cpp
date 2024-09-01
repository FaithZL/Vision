//
// Created by Zero on 2024/8/29.
//

#include "object.h"
#include "serializer.h"

namespace vision::inline hotfix {
SP<ISerialized> RuntimeObject::serialized_data() const noexcept {
    return SerializedData<decltype(this)>::apply(this);
}

void RuntimeObject::restore(vision::RuntimeObject *old_obj) noexcept {
    deserialize(old_obj->serialized_data());
}
}// namespace vision::inline hotfix