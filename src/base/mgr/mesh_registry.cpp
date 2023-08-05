//
// Created by Zero on 2023/8/1.
//

#include "mesh_registry.h"

namespace vision {

MeshRegistry *MeshRegistry::s_mesh_registry = nullptr;

MeshRegistry &MeshRegistry::instance() {
    if (s_mesh_pool == nullptr) {
        s_mesh_pool = new MeshRegistry();
    }
    return *s_mesh_pool;
}

bool MeshRegistry::contain(uint64_t hash) noexcept {
    auto iter = _mesh_map.find(hash);
    return iter != _mesh_map.cend();
}

bool MeshRegistry::contain(const vision::Mesh *mesh) noexcept {
    return contain(mesh->hash());
}

bool MeshRegistry::contain(const SP<const vision::Mesh> &mesh) noexcept {
    return contain(mesh.get());
}

SP<Mesh> MeshRegistry::register_(SP<vision::Mesh> mesh) noexcept {
    uint64_t hash = mesh->hash();
    if (!contain(hash)) {
        _mesh_map.insert(make_pair(hash, mesh));
        _meshes.push_back(mesh.get());
    }
    return mesh;
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
    for (auto iter = _meshes.cbegin();
         iter != _meshes.cend(); ++iter) {
        Mesh *mesh = *iter;
        if (mesh->hash() == hash) {
            _meshes.erase(iter);
            break;
        }
    }
    if (auto iter = _mesh_map.cbegin(); iter != _mesh_map.cend()) {
        _mesh_map.erase(iter);
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
    if (auto iter = _mesh_map.find(hash);
        iter != _mesh_map.cend()) {
        return _mesh_map.at(hash);
    }
    return nullptr;
}

void MeshRegistry::for_each(const std::function<void(Mesh *, uint)> &func) noexcept {
    for (uint i = 0; i < _meshes.size(); ++i) {
        func(_meshes[i], i);
    }
}

void MeshRegistry::for_each(const std::function<void(const Mesh *, uint)> &func) const noexcept {
    for (uint i = 0; i < _meshes.size(); ++i) {
        func(_meshes[i], i);
    }
}

SP<Mesh> MeshRegistry::get_mesh(uint64_t hash) noexcept {
    if (auto iter = _mesh_map.find(hash);
        iter != _mesh_map.cend()) {
        return _mesh_map.at(hash);
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