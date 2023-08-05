//
// Created by Zero on 2023/8/1.
//

#pragma once

#include "base/shape.h"
#include "core/stl.h"

namespace vision {

class MeshPool {
private:
    std::map<uint64_t, SP<Mesh>> _mesh_map;
    vector<Mesh *> _meshes;

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
    [[nodiscard]] bool contain(const SP<const Mesh> &mesh) noexcept;
    [[nodiscard]] bool contain(const Mesh *mesh) noexcept;
    [[nodiscard]] bool contain(uint64_t hash) noexcept;
    [[nodiscard]] SP<Mesh> register_(Mesh mesh) noexcept;
    [[nodiscard]] SP<Mesh> register_(SP<Mesh> mesh) noexcept;
    void for_each(const std::function<void(Mesh *, uint)> &func) noexcept;
    void for_each(const std::function<void(const Mesh *, uint)> &func) const noexcept;
    void tidy_up() noexcept;
    bool remove(SP<Mesh> mesh) noexcept;
    bool remove(uint64_t hash) noexcept;
    bool remove(const Mesh *mesh) noexcept;
};

}// namespace vision