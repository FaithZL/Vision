//
// Created by Zero on 2023/7/14.
//

#pragma once

#include <utility>
#include "core/stl.h"
#include "mgr/scene.h"

namespace vision {

class Importer : public Node {
public:
    using Desc = ImporterDesc;

public:
    explicit Importer(const ImporterDesc &desc) : Node(desc) {}
    static Importer *create(const string &ext_name);
    [[nodiscard]] virtual Scene read_file(const fs::path &fn) = 0;
};

}// namespace vision