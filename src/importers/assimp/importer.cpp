//
// Created by Zero on 2023/7/14.
//

#include "base/importer.h"

namespace vision {

class AssimpImporter : public Importer {
public:
    explicit AssimpImporter(const ImporterDesc &desc)
        : Importer(desc) {}

    [[nodiscard]] Scene read_file(const fs::path &fn) override {
        return {};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AssimpImporter)