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
    _map.insert(make_pair(hash, ocarina::move(mesh)));
}

SP<const Mesh> MeshPool::get_mesh(uint64_t hash) const noexcept {
    if (auto iter = _map.find(hash);
        iter != _map.cend()) {
        return _map.at(hash);
    }
    return nullptr;
}

SP<Mesh> MeshPool::get_mesh(uint64_t hash) noexcept {
    if (auto iter = _map.find(hash);
        iter != _map.cend()) {
        return _map.at(hash);
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