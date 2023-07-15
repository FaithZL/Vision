//
// Created by Zero on 2023/7/14.
//

#include "base/importer.h"

namespace vision {

class JsonImporter : public Importer {
private:
public:
    explicit JsonImporter(const ImporterDesc &desc)
        : Importer(desc) {}

    [[nodiscard]] Scene read_file(const fs::path &fn) override {
        return {};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::JsonImporter)