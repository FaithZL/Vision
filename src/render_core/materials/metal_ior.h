//
// Created by Zero on 26/01/2023.
//

#pragma once

#include "base/color/spd.h"
#include "core/stl.h"

namespace vision {

#include "metal_ior.inl.h"

using namespace ocarina;

struct ComplexIor {
public:
    string name;
    vector<float> eta;
    vector<float> k;

public:
    template<typename TContainer>
    static ComplexIor from(const TContainer &data) {
        ComplexIor ret;
        for (int i = 0; i < data.size(); ++i) {
            ret.eta.push_back(data[i].x);
            ret.k.push_back(data[i].y);
        }
        return ret;
    }
};

class ComplexIorTable {
private:
    map<string, ComplexIor> table_;

private:
    ComplexIorTable() {
#define VS_ADD_METAL_IOR(name) \
    table_.insert(std::make_pair(#name, ComplexIor::from(name)));
        VS_ADD_METAL_IOR(Ag)
        VS_ADD_METAL_IOR(Al)
        VS_ADD_METAL_IOR(Au)
        VS_ADD_METAL_IOR(Cu)
        VS_ADD_METAL_IOR(CuZn)
        VS_ADD_METAL_IOR(Fe)
        VS_ADD_METAL_IOR(Ti)
        VS_ADD_METAL_IOR(V)
        VS_ADD_METAL_IOR(VN)
        VS_ADD_METAL_IOR(Li)
#undef VS_ADD_METAL_IOR
    }

public:
    static ComplexIorTable *instance() noexcept {
        static ComplexIorTable complex_ior_table;
        return &complex_ior_table;
    }

    [[nodiscard]] vector<const char *> all_keys() const noexcept {
        static vector<const char *> ret = [this]() {
            vector<const char *> keys;
            for (const auto& pair : table_) {
                keys.push_back(pair.first.c_str());
            }
            return keys;
        }();

        return ret;
    }

    [[nodiscard]] const ComplexIor &get_ior(const string& name) const noexcept {
        if (auto iter = table_.find(name); iter == table_.end()) {
            return table_.at("Ag");
        }
        return table_.at(name);
    }
};

}// namespace vision
