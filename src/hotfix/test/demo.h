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
private:
    SP<Test> test;
    int attr_int{1};
    float attr_float{2.5};

public:
    Demo();
    void clear() noexcept {
        test->clear();
        attr_int = 0;
        attr_float = 0;
    }

    void on_update(const vector<const IObjectConstructor *> &constructors) noexcept override;

    [[nodiscard]] string get_string() const;

    void fill() noexcept {
        test->fill();
        attr_float = 57.9;
        attr_int = 789;
    }

    void print() noexcept {
        std::cout << "demo print begin" << std::endl;
        std::cout << "      attr_float = " << attr_float << endl;
        std::cout << "      attr_int = " << attr_int << endl;
        test->print();
        std::cout << "demo print end" << std::endl;
    }
    void serialize(SP<ISerialized> output) const noexcept override;
    void deserialize(SP<ISerialized> input) noexcept override;
};

class HotfixTest : public Observer {
public:
    SP<Demo> demo{make_shared<Demo>()};
    SP<Test> test{make_shared<Test>()};

public:
    void on_update(const vector<const IObjectConstructor *> &constructors) noexcept override {
    }
};

}// namespace vision::inline hotfix