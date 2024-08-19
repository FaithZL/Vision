//
// Created by Zero on 2024/8/19.
//

#include "hotfix/compiler.h"

namespace vision::inline hotfix {

class LLVMCompiler : public Compiler {
public:
    [[nodiscard]] string get_object_file_extension() const noexcept override {
        return "o";
    }
};

}// namespace vision::inline hotfix