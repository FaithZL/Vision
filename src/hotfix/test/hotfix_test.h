//
// Created by Zero on 2024/8/30.
//

#pragma once

#include "demo.h"

namespace vision::inline hotfix {

class HotfixTest : public Observer {
public:
    SP<Demo> demo{make_shared<Demo>()};
    SP<Test> test{make_shared<Test>()};

public:
    void update_runtime_object(const IObjectConstructor *constructor) noexcept override;
};
}// namespace vision::inline hoftix