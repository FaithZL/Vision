//
// Created by Zero on 23/01/2023.
//

#include "shader_node.h"
#include "base/mgr/pipeline.h"
#include "GUI/widgets.h"

namespace vision {

SlotBase::SlotBase(int, std::string channels)
    : dim_(channels.size()),
#ifndef NDEBUG
      channels_(channels),
#endif
      channel_mask_(calculate_mask(ocarina::move(channels))) {
}

uint SlotBase::calculate_mask(string channels) noexcept {
    uint ret{};
    channels = to_lower(channels);
    static map<char, uint> dict{
        {'x', 0u},
        {'y', 1u},
        {'z', 2u},
        {'w', 3u},
        {'r', 0u},
        {'g', 1u},
        {'b', 2u},
        {'a', 3u},
    };
    for (char channel : channels) {
        ret = (ret << 4) | dict[channel];
    }
    return ret;
}

Slot::Slot(SP<vision::ShaderNode> input, std::string channels)
    : SlotBase(0, channels), node_(std::move(input)) {
    OC_ASSERT(dim_ <= 4);
}

Slot Slot::create_slot(const vision::SlotDesc &desc) {
    SP<ShaderNode> shader_node = Node::create_shared<ShaderNode>(desc.node);
    return Slot(shader_node, desc.channels);
}

Slot &Slot::set(const vision::Slot &other) noexcept {
    string old_name = attr_name_;
    *this = other;
    if (other.attr_name_.empty()) {
        attr_name_ = old_name;
    }
    return *this;
}

void Slot::update_runtime_object(const vision::IObjectConstructor *constructor) noexcept {
    if (node_) {
        HotfixSystem::replace_objects(constructor, std::tuple{addressof(node_)});
    }
}
void Slot::reset_status() noexcept {
    if (node_) {
        node_->reset_status();
    }
}
bool Slot::has_changed() noexcept {
    if (node_) {
        return node_->has_changed();
    }
    return false;
}
void Slot::render_sub_UI(ocarina::Widgets *widgets) noexcept {
    if (node_) {
        node_->render_sub_UI(widgets);
    }
}

bool Slot::render_UI(ocarina::Widgets *widgets) noexcept {
    if (node_) {
        if (!attr_name_.empty()) {
            node_->set_name(attr_name_);
        }
        return node_->render_UI(widgets);
    }
    return false;
}

uint64_t Slot::_compute_hash() const noexcept {
    return hash64(channel_mask_, dim_, node_->hash(), node_->node_tag());
}

uint64_t Slot::_compute_type_hash() const noexcept {
    return hash64(channel_mask_, dim_, node_->type_hash(), node_->node_tag());
}

DynamicArray<float> Slot::evaluate(const AttrEvalContext &ctx,
                                   const SampledWavelengths &swl) const noexcept {
    switch (dim_) {
        case 1: {
            switch (channel_mask_) {
#include "slot_swizzle_1.inl.h"
            }
        }
        case 2: {
            switch (channel_mask_) {
#include "slot_swizzle_2.inl.h"
            }
        }
        case 3: {
            switch (channel_mask_) {
#include "slot_swizzle_3.inl.h"
            }
        }
        case 4: {
            switch (channel_mask_) {
#include "slot_swizzle_4.inl.h"
            }
        }
    }
    return node_->evaluate(ctx, swl);
}

vector<float> Slot::average() const noexcept {
    switch (dim_) {
        case 1: {
            switch (channel_mask_) {
#include "slot_average_swizzle_1.inl.h"
            }
        }
        case 2: {
            switch (channel_mask_) {
#include "slot_average_swizzle_2.inl.h"
            }
        }
        case 3: {
            switch (channel_mask_) {
#include "slot_average_swizzle_3.inl.h"
            }
        }
        case 4: {
            switch (channel_mask_) {
#include "slot_average_swizzle_4.inl.h"
            }
        }
    }
    OC_ASSERT(false);
    return {};
}

float Slot::luminance() const noexcept {
    switch (dim_) {
        case 1: return average()[0];
        case 2:
            OC_ERROR("float2 not is a color attribute !");
            return 0;
        case 3:
        case 4:
            auto a = average();
            return ocarina::luminance(make_float3(a[0], a[1], a[2]));
    }
    OC_ASSERT(false);
    return 0;
}

ColorDecode Slot::eval_albedo_spectrum(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(dim_ == 3);
    Float3 val = evaluate(ctx, swl).as_vec3();
    return node_->spectrum()->decode_to_albedo(val, swl);
}

ColorDecode Slot::eval_unbound_spectrum(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(dim_ == 3);
    Float3 val = evaluate(ctx, swl).as_vec3();
    return node_->spectrum()->decode_to_unbound_spectrum(val, swl);
}

ColorDecode Slot::eval_illumination_spectrum(const AttrEvalContext &ctx, const SampledWavelengths &swl) const noexcept {
    OC_ASSERT(dim_ == 3);
    Float3 val = evaluate(ctx, swl).as_vec3();
    return node_->spectrum()->decode_to_illumination(val, swl);
}

}// namespace vision