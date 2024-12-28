//
// Created by Zero on 2023/8/1.
//

#include "mesh_registry.h"

namespace vision {

MeshRegistry *MeshRegistry::s_mesh_registry = nullptr;

MeshRegistry &MeshRegistry::instance() {
    if (s_mesh_registry == nullptr) {
        s_mesh_registry = new MeshRegistry();
        HotfixSystem::instance().register_static_var("MeshRegistry", *s_mesh_registry);
    }
    return *s_mesh_registry;
}

bool MeshRegistry::contain(uint64_t hash) noexcept {
    auto iter = mesh_map_.find(hash);
    return iter != mesh_map_.cend();
}

bool MeshRegistry::contain(const vision::Mesh *mesh) noexcept {
    return contain(mesh->hash());
}

bool MeshRegistry::contain(const SP<const vision::Mesh> &mesh) noexcept {
    return contain(mesh.get());
}

CommandList MeshRegistry::upload_meshes() noexcept {
    CommandList ret;
    for_each([&](Mesh *mesh, uint i) {
        ret << mesh->upload();
    });
    return ret;
}

SP<Mesh> MeshRegistry::register_(SP<vision::Mesh> mesh) noexcept {
    uint64_t hash = mesh->hash();
    if (!contain(hash)) {
        mesh_map_.insert(make_pair(hash, mesh));
        meshes_.push_back(mesh.get());
    }
    return get_mesh(hash);
}

SP<Mesh> MeshRegistry::register_(vision::Mesh mesh) noexcept {
    uint64_t hash = mesh.hash();
    if (!contain(hash)) {
        return register_(make_shared<Mesh>(ocarina::move(mesh)));
    }
    return get_mesh(hash);
}

bool MeshRegistry::remove(SP<vision::Mesh> mesh) noexcept {
    return remove(mesh.get());
}

bool MeshRegistry::remove(const vision::Mesh *mesh) noexcept {
    return remove(mesh->hash());
}

bool MeshRegistry::remove(uint64_t hash) noexcept {
    for (auto iter = meshes_.cbegin();
         iter != meshes_.cend(); ++iter) {
        Mesh *mesh = *iter;
        if (mesh->hash() == hash) {
            meshes_.erase(iter);
            break;
        }
    }
    if (auto iter = mesh_map_.cbegin(); iter != mesh_map_.cend()) {
        mesh_map_.erase(iter);
        return true;
    }
    return false;
}

void MeshRegistry::tidy_up() noexcept {
    for_each([&](Mesh *mesh, uint i) {
        mesh->set_index(i);
    });
}

SP<const Mesh> MeshRegistry::get_mesh(uint64_t hash) const noexcept {
    if (auto iter = mesh_map_.find(hash);
        iter != mesh_map_.cend()) {
        return mesh_map_.at(hash);
    }
    return nullptr;
}

void MeshRegistry::clear() noexcept {
    mesh_map_.clear();
    meshes_.clear();
}

void MeshRegistry::for_each(const std::function<void(Mesh *, uint)> &func) noexcept {
    for (uint i = 0; i < meshes_.size(); ++i) {
        func(meshes_[i], i);
    }
}

void MeshRegistry::for_each(const std::function<void(const Mesh *, uint)> &func) const noexcept {
    for (uint i = 0; i < meshes_.size(); ++i) {
        func(meshes_[i], i);
    }
}

SP<Mesh> MeshRegistry::get_mesh(uint64_t hash) noexcept {
    if (auto iter = mesh_map_.find(hash);
        iter != mesh_map_.cend()) {
        return mesh_map_.at(hash);
    }
    return nullptr;
}

void MeshRegistry::destroy_instance() {
    if (s_mesh_registry) {
        delete s_mesh_registry;
        s_mesh_registry = nullptr;
    }
}

}// namespace vision