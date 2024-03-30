//
// Created by Zero on 2024/3/17.
//

#pragma once

#include "GUI/decl.h"
#include "core/element_trait.h"
#include "math/transform.h"

namespace ocarina {
class Widgets;
}

#define VS_MAKE_RENDER_UI(obj) vision::UI::render_UI(obj, widgets);
#define VS_MAKE_RESET_STATUS(obj) vision::UI::reset_status(obj);
#define VS_MAKE_HAS_CHANGED(obj) ret |= vision::UI::has_changed(obj);

#define VS_MAKE_GUI_STATUS_FUNC(Super, ...)      \
    void reset_status() noexcept override {      \
        Super::reset_status();                   \
        MAP(VS_MAKE_RESET_STATUS, ##__VA_ARGS__) \
    }                                            \
    bool has_changed() noexcept override {       \
        bool ret = Super::_changed;              \
        MAP(VS_MAKE_HAS_CHANGED, ##__VA_ARGS__)  \
        return ret;                              \
    }

#define VS_MAKE_GUI_ALL_FUNC(Super, ...)                          \
    VS_MAKE_GUI_STATUS_FUNC(Super, ##__VA_ARGS__)                 \
    bool render_UI(ocarina::Widgets *widgets) noexcept override { \
        widgets->use_window("scene data", [&] {                   \
            MAP(VS_MAKE_RENDER_UI, ##__VA_ARGS__)                 \
        });                                                       \
        return true;                                              \
    }

namespace vision {

namespace UI {

OC_MAKE_AUTO_MEMBER_FUNC(reset_status)
OC_MAKE_AUTO_MEMBER_FUNC(has_changed)
OC_MAKE_AUTO_MEMBER_FUNC(render_UI)
OC_MAKE_AUTO_MEMBER_FUNC(render_sub_UI)

}// namespace UI

class GUI {
protected:
    bool _changed{false};

public:
    virtual void reset_status() noexcept { _changed = false; }
    [[nodiscard]] virtual bool has_changed() noexcept { return _changed; }

    /**
     * @param widgets
     * @return Status of the widget switch
     */
    virtual bool render_UI(ocarina::Widgets *widgets) noexcept { return true; }

    /**
     * @param widgets
     * @return Whether any data has been updated
     */
    virtual void render_sub_UI(ocarina::Widgets *widgets) noexcept {}
    virtual ~GUI() = default;
};

class TransformWidget : public GUI, public Serializable<> {
private:
    string _name;
    Serial<float4x4> _transform;
    float3 _t;
    float4 _r;
    float3 _s;

public:
    explicit TransformWidget(float4x4 m) : _transform(m) {
        quaternion q;
        decompose(_transform.hv(), addressof(_t), addressof(q), addressof(_s));
        _r = make_float4(q.axis(), degrees(q.theta()));
    }
    OC_SERIALIZABLE_FUNC(Serializable<>,_transform)
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
};

}// namespace vision