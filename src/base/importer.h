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
    static SP<Importer> create(const string &ext_name);
    static void import_scene(const fs::path &fn, Scene *scene);
    virtual void read_file(const fs::path &fn, Scene *scene) = 0;
};

}// namespace vision