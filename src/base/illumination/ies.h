//
// Created by Zero on 2023/8/20.
//

#pragma once

#include "core/stl.h"

namespace vision {

using namespace ocarina;

class IESFile {
protected:
    /**
     * The brightness distribution is stored in spherical coordinates.
     * The horizontal angles correspond to theta in the regular notation
     * and always span the full range from 0бу to 360бу.
     * The vertical angles correspond to phi and always start at 0бу.
     */
    vector<float> v_angles, h_angles;

    /**
     * The actual values are stored here, with every entry storing the values
     * of one horizontal segment.
     */
    vector<vector<float>> intensity;

    /**
     * Types of angle representation in IES files. Currently, only B and C are supported.
     */
    enum IESType { TYPE_A = 3,
                   TYPE_B = 2,
                   TYPE_C = 1 } type;

public:
    IESFile() = default;
    ~IESFile();
    int packed_size();
    void pack(float *data);
    bool load(const string &ies);
    void clear();

protected:
    bool parse(const string &ies);
    bool process();
    bool process_type_b();
    bool process_type_c();
};

}// namespace vision