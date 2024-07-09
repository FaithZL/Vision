//
// Created by Zero on 2024/7/29.
//

#pragma once

#include "core/stl.h"
#include "core/hash.h"
#include "serializer.h"

namespace vision::inline hotfix {

using namespace ocarina;

class RuntimeObject : public Hashable {
public:
    virtual void serialize(Serializer *serializer) const noexcept = 0;
    virtual void deserialize(Serializer *serializer) const noexcept = 0;
};

class RuntimeObjectMgr {
private:
    RuntimeObjectMgr() = default;
    ~RuntimeObjectMgr();
    static RuntimeObjectMgr *s_mgr;

private:
    using ObjectGroup = vector<SP<RuntimeObject>>;
    map<string, ObjectGroup> map_;
    Serializer serializer_;

public:
    RuntimeObjectMgr(const RuntimeObjectMgr &) = delete;
    RuntimeObjectMgr(RuntimeObjectMgr &&) = delete;
    RuntimeObjectMgr operator=(const RuntimeObjectMgr &) = delete;
    RuntimeObjectMgr operator=(RuntimeObjectMgr &&) = delete;
    void add_object(SP<RuntimeObject> object) noexcept;
    void update(SP<RuntimeObject> object) noexcept {
        update(object->class_name());
    }
    void update(const string &c_name) noexcept;
    void remove_object(SP<RuntimeObject> object) noexcept;
    OC_MAKE_MEMBER_GETTER(serializer, &)
    static RuntimeObjectMgr &instance() noexcept;
    static void destroy_instance() noexcept;
};

}// namespace vision::inline hotfix
