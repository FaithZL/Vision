//
// Created by Zero on 2023/8/1.
//

#include "mesh_pool.h"

namespace vision {

MeshPool *MeshPool::s_mesh_pool = nullptr;

MeshPool &MeshPool::instance() {
    if (s_mesh_pool == nullptr) {
        s_mesh_pool = new MeshPool();
    }
    return *s_mesh_pool;
}

void MeshPool::add_mesh(SP<vision::Mesh> mesh) noexcept {
    uint64_t hash = mesh->hash();
    add_mesh(hash, ocarina::move(mesh));
}

void MeshPool::add_mesh(uint64_t hash, SP<vision::Mesh> mesh) noexcept {
    _meshes.push_back(mesh.get());
    _mesh_map.insert(make_pair(hash, ocarina::move(mesh)));
}

bool MeshPool::remove(SP<vision::Mesh> mesh) noexcept {
    return remove(mesh.get());
}

bool MeshPool::remove(const vision::Mesh *mesh) noexcept {
    return remove(mesh->hash());
}

bool MeshPool::remove(uint64_t hash) noexcept {
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

void MeshPool::tidy_up() noexcept {
    for_each([&](Mesh *mesh, uint i) {
        mesh->set_index(i);
    });
}

SP<const Mesh> MeshPool::get_mesh(uint64_t hash) const noexcept {
    if (auto iter = _mesh_map.find(hash);
        iter != _mesh_map.cend()) {
        return _mesh_map.at(hash);
    }
    return nullptr;
}

void MeshPool::for_each(const std::function<void(Mesh *, uint)> &func) noexcept {
    for (uint i = 0; i < _meshes.size(); ++i) {
        func(_meshes[i], i);
    }
}

void MeshPool::for_each(const std::function<void(const Mesh *, uint)> &func) const noexcept {
    for (uint i = 0; i < _meshes.size(); ++i) {
        func(_meshes[i], i);
    }
}

SP<Mesh> MeshPool::get_mesh(uint64_t hash) noexcept {
    if (auto iter = _mesh_map.find(hash);
        iter != _mesh_map.cend()) {
        return _mesh_map.at(hash);
    }
    return nullptr;
}

void MeshPool::destroy_instance() {
    if (s_mesh_pool) {
        delete s_mesh_pool;
        s_mesh_pool = nullptr;
    }
}

}// namespace vision