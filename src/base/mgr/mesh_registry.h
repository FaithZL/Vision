//
// Created by Zero on 2023/8/1.
//

#pragma once

#include "base/shape.h"
#include "core/stl.h"

namespace vision {

class MeshRegistry {
private:
    std::map<uint64_t, SP<Mesh>> mesh_map_;
    vector<Mesh *> meshes_;

private:
    static MeshRegistry *s_mesh_registry;
    MeshRegistry() = default;
    MeshRegistry(const MeshRegistry &) = delete;
    MeshRegistry(MeshRegistry &&) = delete;
    MeshRegistry operator=(const MeshRegistry &) = delete;
    MeshRegistry operator=(MeshRegistry &&) = delete;

public:
    static MeshRegistry &instance();
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
    void clear() noexcept;
    bool remove(SP<Mesh> mesh) noexcept;
    bool remove(uint64_t hash) noexcept;
    bool remove(const Mesh *mesh) noexcept;
};

}// namespace vision