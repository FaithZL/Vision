//
// Created by Zero on 05/10/2022.
//

#pragma once

#include "core/stl.h"
#include "rhi/context.h"
#include "descriptions/node_desc.h"
#include "cli_parser.h"
#include "base/node.h"
#include "scene.h"
#include "descriptions/node_desc.h"

namespace vision {

using namespace ocarina;

class Filter;

class Context : public ocarina::Context {
public:
    using Super = ocarina::Context;

private:
    CLIParser _cli_parser;
    vector<Node::Handle> _all_nodes;
    Scene _scene{this};

public:
    explicit Context(int argc, char **argv,
                     ocarina::string_view cache_dir = ".cache");
    void prepare() noexcept;
    [[nodiscard]] const CLIParser &cli_parser() const noexcept { return _cli_parser; }
    [[nodiscard]] CLIParser &cli_parser() noexcept { return _cli_parser; }
    [[nodiscard]] const Scene *scene() const noexcept { return &_scene; }
    [[nodiscard]] Scene *scene() noexcept { return &_scene; }
    [[nodiscard]] Node *load_node(const NodeDesc *desc);
    template<typename T, typename desc_ty>
    [[nodiscard]] T *load(const desc_ty *desc) noexcept {
        auto ret = dynamic_cast<T*>(load_node(desc));
        OC_ERROR_IF(ret == nullptr, "error node load ", desc->name);
        return ret;
    }
    [[nodiscard]] Filter *load_filter(const FilterDesc *desc);
};

}// namespace vision