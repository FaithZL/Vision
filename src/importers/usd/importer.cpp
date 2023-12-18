//
// Created by Zero on 2023/7/14.
//

#include "usd_reader.h"
#include "base/importer.h"

namespace vision {
class USDImporter : public Importer {
public:
    explicit USDImporter(const ImporterDesc &desc)
        : Importer(desc) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    [[nodiscard]] SP<Pipeline> read_file(const fs::path &fn) override {
        return nullptr;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::USDImporter)
