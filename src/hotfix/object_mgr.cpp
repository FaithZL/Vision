//
// Created by Zero on 2024/7/29.
//

#include "object_mgr.h"

namespace vision ::inline hotfix {

RuntimeObjectMgr *RuntimeObjectMgr::s_mgr = nullptr;

void RuntimeObjectMgr::add_object(SP<vision::RuntimeObject> object) noexcept {
    string c_name = object->class_name();
    if (!map_.contains(c_name)) {
        map_.insert(make_pair(c_name, ObjectGroup{}));
    }
    map_[c_name].push_back(std::move(object));
}

void RuntimeObjectMgr::remove_object(SP<vision::RuntimeObject> object) noexcept {
    string c_name = object->class_name();
    if (!map_.contains(c_name)) {
        return;
    }
    ObjectGroup &group = map_[c_name];
    auto iter = std::find_if(group.begin(), group.end(), [&](SP<RuntimeObject> element) {
        return element.get() == object.get();
    });
    if (iter == group.end()) {
        return;
    }
    group.erase(iter);
}

void RuntimeObjectMgr::update(const std::string &c_name) noexcept {
    if (!map_.contains(c_name)) {
        return;
    }
    ObjectGroup &group = map_[c_name];

}

RuntimeObjectMgr &RuntimeObjectMgr::instance() noexcept {
    if (s_mgr == nullptr) {
        s_mgr = new RuntimeObjectMgr();
    }
    return *s_mgr;
}

void RuntimeObjectMgr::destroy_instance() noexcept {
    if (s_mgr == nullptr) {
        return;
    }
    delete s_mgr;
    s_mgr = nullptr;
}

RuntimeObjectMgr::~RuntimeObjectMgr() {}

}// namespace vision::inline hotfix