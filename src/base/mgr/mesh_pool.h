//
// Created by Zero on 2023/8/1.
//

#pragma once

#include "base/shape.h"
#include "core/stl.h"

namespace vision {

class MeshPool {
private:
    std::map<uint64_t, SP<Mesh>> _map;

private:
    static MeshPool *s_mesh_pool;
    MeshPool() = default;
    MeshPool(const MeshPool &) = delete;
    MeshPool(MeshPool &&) = delete;
    MeshPool operator=(const MeshPool &) = delete;
    MeshPool operator=(MeshPool &&) = delete;

public:
    static MeshPool &instance();
    static void destroy_instance();
    [[nodiscard]] SP<const Mesh> get_mesh(uint64_t hash) const noexcept;
    [[nodiscard]] SP<Mesh> get_mesh(uint64_t hash) noexcept;
    void add_mesh(uint64_t hash, SP<Mesh> mesh) noexcept;
    void add_mesh(SP<Mesh> mesh) noexcept;
};

}// namespace vision