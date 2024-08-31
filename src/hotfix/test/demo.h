//
// Created by Zero on 2024/8/21.
//

#pragma once

#include "core/stl.h"
#include "hotfix/object.h"
#include "hotfix/hotfix.h"
#include "test.h"

namespace vision::inline hotfix {

struct Demo : public RuntimeObject, public Observer {
public:
    SP<Test> test;
    int attr_int{};
    float attr_float{};

public:
    Demo();
    virtual void clear() noexcept;
    void update_runtime_object(const IObjectConstructor *constructor) noexcept override;
    void restore(const vision::RuntimeObject *old_obj) noexcept override;
    [[nodiscard]] virtual string get_string() const;
    virtual void fill() noexcept;
    virtual void print() noexcept;
    void serialize(SP<ISerialized> output) const noexcept override;
    void deserialize(SP<ISerialized> input) noexcept override;
};


}// namespace vision::inline hotfix