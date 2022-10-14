//
// Created by Zero on 13/10/2022.
//

#pragma once

#include "core/stl.h"
#include "core/basic_types.h"
#include "core/header.h"

namespace vision {
using namespace ocarina;
class Desc {
public:
    enum Tag {
        ENone,
        ESampler,
        EFilter,
        EFilm,
        ESensor,
        EIntegrator,
        ETexture,
        EMaterial,
        ELight,
        ETransform,
        ELightSampler
    };
    using bool_ty = bool;
    using number_ty = double;
    using string_ty = ocarina::string;
    using node_ty = unique_ptr<Desc>;
    using map_ty = ocarina::unordered_map<string_ty, node_ty>;
    using list_ty = vector<node_ty>;
    using value_ty = ocarina::variant<bool_ty, number_ty, string_ty, node_ty, map_ty, list_ty>;

private:
    Tag _tag{};
    value_ty _value;

public:
    Desc(const DataWrap &data) { init(data); }
    void init(const DataWrap &data) {}
    template<typename... Args>
    static Desc from_json(Args &&...args) {
        return Desc(OC_FORWARD(args)...);
    }
};

}// namespace vision