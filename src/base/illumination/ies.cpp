//
// Created by Zero on 2023/8/20.
//

#include "ies.h"

namespace vision {

class IESTextParser {
public:
    vector<char> text;
    char *data;
    bool error;

    IESTextParser(const string &str) : text(str.begin(), str.end()), error(false) {
        std::replace(text.begin(), text.end(), ',', ' ');
        data = strstr(&text[0], "\nTILT=");
    }

    bool eof() { return (data == nullptr) || (data[0] == '\0'); }

    bool has_error() { return error; }

    double get_double() {
        if (eof()) {
            error = true;
            return 0.0;
        }
        char *old_data = data;
        double val = strtod(data, &data);
        if (data == old_data) {
            data = NULL;
            error = true;
            return 0.0;
        }
        return val;
    }

    long get_long() {
        if (eof()) {
            error = true;
            return 0;
        }
        char *old_data = data;
        long val = strtol(data, &data, 10);
        if (data == old_data) {
            data = NULL;
            error = true;
            return 0;
        }
        return val;
    }
};

bool IESFile::load(const string &ies) {
    clear();
    if (!parse(ies) || !process()) {
        clear();
        return false;
    }
    return true;
}

void IESFile::clear() {
    intensity.clear();
    v_angles.clear();
    h_angles.clear();
}

int IESFile::packed_size() {
    if (v_angles.size() && h_angles.size() > 0) {
        return 2 + h_angles.size() + v_angles.size() + h_angles.size() * v_angles.size();
    }
    return 0;
}

void IESFile::pack(float *data) {
    if (v_angles.empty() || h_angles.empty()) {
        return ;
    }
    *(data++) = bit_cast<float>(int(h_angles.size()));
    *(data++) = bit_cast<float>(int(v_angles.size()));

    memcpy(data, &h_angles[0], h_angles.size() * sizeof(float));
    data += h_angles.size();
    memcpy(data, &v_angles[0], v_angles.size() * sizeof(float));
    data += v_angles.size();

    for (int h = 0; h < intensity.size(); h++) {
        memcpy(data, &intensity[h][0], v_angles.size() * sizeof(float));
        data += v_angles.size();
    }
}

}// namespace vision