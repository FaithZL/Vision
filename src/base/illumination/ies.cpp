//
// Created by Zero on 2023/8/20.
//

#include "ies.h"
#include "core/basic_types.h"
#include "core/constants.h"

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
    _intensity.clear();
    _v_angles.clear();
    _h_angles.clear();
}

int IESFile::packed_size() {
    if (_v_angles.size() && _h_angles.size() > 0) {
        return 2 + _h_angles.size() + _v_angles.size() + _h_angles.size() * _v_angles.size();
    }
    return 0;
}

void IESFile::pack(float *data) {
    if (_v_angles.empty() || _h_angles.empty()) {
        return;
    }
    *(data++) = bit_cast<float>(int(_h_angles.size()));
    *(data++) = bit_cast<float>(int(_v_angles.size()));

    memcpy(data, &_h_angles[0], _h_angles.size() * sizeof(float));
    data += _h_angles.size();
    memcpy(data, &_v_angles[0], _v_angles.size() * sizeof(float));
    data += _v_angles.size();

    for (int h = 0; h < _intensity.size(); h++) {
        memcpy(data, &_intensity[h][0], _v_angles.size() * sizeof(float));
        data += _v_angles.size();
    }
}

bool IESFile::parse(const string &ies) {
    if (ies.empty()) {
        return false;
    }

    IESTextParser parser(ies);
    if (parser.eof()) {
        return false;
    }

    /* Handle the tilt data block. */
    if (strncmp(parser.data, "\nTILT=INCLUDE", 13) == 0) {
        parser.data += 13;
        parser.get_double();              /* Lamp to Luminaire geometry */
        int num_tilt = parser.get_long(); /* Amount of tilt angles and factors */
        /* Skip over angles and factors. */
        for (int i = 0; i < 2 * num_tilt; i++) {
            parser.get_double();
        }
    } else {
        /* Skip to next line. */
        parser.data = strstr(parser.data + 1, "\n");
    }

    if (parser.eof()) {
        return false;
    }
    parser.data++;

    parser.get_long();                    /* Number of lamps */
    parser.get_double();                  /* Lumens per lamp */
    double factor = parser.get_double();  /* Candela multiplier */
    int v_angles_num = parser.get_long(); /* Number of vertical angles */
    int h_angles_num = parser.get_long(); /* Number of horizontal angles */
    type = (IESType)parser.get_long();    /* Photometric type */

    /* TODO(lukas): Test whether the current type B processing can also deal with type A files.
   * In theory the only difference should be orientation which we ignore anyways, but with IES you
   * never know...
   */
    if (type != TYPE_B && type != TYPE_C) {
        return false;
    }

    parser.get_long();             /* Unit of the geometry data */
    parser.get_double();           /* Width */
    parser.get_double();           /* Length */
    parser.get_double();           /* Height */
    factor *= parser.get_double(); /* Ballast factor */
    factor *= parser.get_double(); /* Ballast-Lamp Photometric factor */
    parser.get_double();           /* Input Watts */

    /**
     * Intensity values in IES files are specified in candela (lumen/sr), a photometric quantity.
     * Cycles expects radiometric quantities, though, which requires a conversion.
     * However, the Luminous efficacy (ratio of lumens per Watt) depends on the spectral distribution
     * of the light source since lumens take human perception into account.
     * Since this spectral distribution is not known from the IES file, a typical one must be
     * assumed. The D65 standard illuminant has a Luminous efficacy of 177.83, which is used here to
     * convert to Watt/sr. A more advanced approach would be to add a Blackbody Temperature input to
     * the node and numerically integrate the Luminous efficacy from the resulting spectral
     * distribution. Also, the Watt/sr value must be multiplied by 4*pi to get the Watt value that
     * Cycles expects for lamp strength. Therefore, the conversion here uses 4*pi/177.83 as a Candela
     * to Watt factor.
     */
    factor *= 0.0706650768394;

    _v_angles.reserve(v_angles_num);
    for (int i = 0; i < v_angles_num; i++) {
        _v_angles.push_back((float)parser.get_double());
    }

    _h_angles.reserve(h_angles_num);
    for (int i = 0; i < h_angles_num; i++) {
        _h_angles.push_back((float)parser.get_double());
    }

    _intensity.resize(h_angles_num);
    for (int i = 0; i < h_angles_num; i++) {
        _intensity[i].reserve(v_angles_num);
        for (int j = 0; j < v_angles_num; j++) {
            _intensity[i].push_back((float)(factor * parser.get_double()));
        }
    }

    return !parser.has_error();
}

bool IESFile::process_type_b() {
    vector<vector<float>> newintensity;
    newintensity.resize(_v_angles.size());
    for (int i = 0; i < _v_angles.size(); i++) {
        newintensity[i].reserve(_h_angles.size());
        for (int j = 0; j < _h_angles.size(); j++) {
            newintensity[i].push_back(_intensity[j][i]);
        }
    }
    _intensity.swap(newintensity);
    _h_angles.swap(_v_angles);

    float h_first = _h_angles[0], h_last = _h_angles[_h_angles.size() - 1];
    if (h_last != 90.0f) {
        return false;
    }

    if (h_first == 0.0f) {
        /**
         * The range in the file corresponds to 90бу-180бу, we need to mirror that to get the
         * full 180бу range.
         */
        vector<float> new_h_angles;
        vector<vector<float>> new_intensity;
        int hnum = _h_angles.size();
        new_h_angles.reserve(2 * hnum - 1);
        new_intensity.reserve(2 * hnum - 1);
        for (int i = hnum - 1; i > 0; i--) {
            new_h_angles.push_back(90.0f - _h_angles[i]);
            new_intensity.push_back(_intensity[i]);
        }
        for (int i = 0; i < hnum; i++) {
            new_h_angles.push_back(90.0f + _h_angles[i]);
            new_intensity.push_back(_intensity[i]);
        }
        _h_angles.swap(new_h_angles);
        _intensity.swap(new_intensity);
    } else if (h_first == -90.0f) {
        /* We have full 180бу coverage, so just shift to match the angle range convention. */
        for (int i = 0; i < _h_angles.size(); i++) {
            _h_angles[i] += 90.0f;
        }
    }
    /**
     * To get correct results with the cubic interpolation in the kernel, the horizontal range
     * has to cover all 360бу. Therefore, we copy the 0бу entry to 360бу to ensure full coverage
     * and seamless interpolation.
     */
    _h_angles.push_back(360.0f);
    _intensity.push_back(_intensity[0]);

    float v_first = _v_angles[0], v_last = _v_angles[_v_angles.size() - 1];
    if (v_last != 90.0f) {
        return false;
    }

    if (v_first == 0.0f) {
        /** The range in the file corresponds to 90бу-180бу, we need to mirror that to get the
         * full 180бу range.
         */
        vector<float> new_v_angles;
        int hnum = _h_angles.size();
        int vnum = _v_angles.size();
        new_v_angles.reserve(2 * vnum - 1);
        for (int i = vnum - 1; i > 0; i--) {
            new_v_angles.push_back(90.0f - _v_angles[i]);
        }
        for (int i = 0; i < vnum; i++) {
            new_v_angles.push_back(90.0f + _v_angles[i]);
        }
        for (int i = 0; i < hnum; i++) {
            vector<float> new_intensity;
            new_intensity.reserve(2 * vnum - 1);
            for (int j = vnum - 2; j >= 0; j--) {
                new_intensity.push_back(_intensity[i][j]);
            }
            new_intensity.insert(new_intensity.end(), _intensity[i].begin(), _intensity[i].end());
            _intensity[i].swap(new_intensity);
        }
        _v_angles.swap(new_v_angles);
    } else if (v_first == -90.0f) {
        /* We have full 180бу coverage, so just shift to match the angle range convention. */
        for (int i = 0; i < _v_angles.size(); i++) {
            _v_angles[i] += 90.0f;
        }
    }

    return true;
}

bool IESFile::process_type_c() {
    if (_h_angles[0] == 90.0f) {
        /* Some files are stored from 90бу to 270бу, so we just rotate them to the regular 0бу-180бу range
     * here. */
        for (int i = 0; i < _h_angles.size(); i++) {
            _h_angles[i] -= 90.0f;
        }
    }

    if (_h_angles[0] != 0.0f) {
        return false;
    }

    if (_h_angles.size() == 1) {
        _h_angles.push_back(360.0f);
        _intensity.push_back(_intensity[0]);
    }

    if (_h_angles[_h_angles.size() - 1] == 90.0f) {
        /**
         * Only one quadrant is defined, so we need to mirror twice (from one to two, then to four).
         * Since the two->four mirroring step might also be required if we get an input of two
         * quadrants, we only do the first mirror here and later do the second mirror in either case.
         */
        int hnum = _h_angles.size();
        for (int i = hnum - 2; i >= 0; i--) {
            _h_angles.push_back(180.0f - _h_angles[i]);
            _intensity.push_back(_intensity[i]);
        }
    }

    if (_h_angles[_h_angles.size() - 1] == 180.0f) {
        /* Mirror half to the full range. */
        int hnum = _h_angles.size();
        for (int i = hnum - 2; i >= 0; i--) {
            _h_angles.push_back(360.0f - _h_angles[i]);
            _intensity.push_back(_intensity[i]);
        }
    }

    /* Some files skip the 360бу entry (contrary to standard) because it's supposed to be identical to
   * the 0бу entry. If the file has a discernible order in its spacing, just fix this. */
    if (_h_angles[_h_angles.size() - 1] != 360.0f) {
        int hnum = _h_angles.size();
        float last_step = _h_angles[hnum - 1] - _h_angles[hnum - 2];
        float first_step = _h_angles[1] - _h_angles[0];
        float difference = 360.0f - _h_angles[hnum - 1];
        if (last_step == difference || first_step == difference) {
            _h_angles.push_back(360.0f);
            _intensity.push_back(_intensity[0]);
        } else {
            return false;
        }
    }

    float v_first = _v_angles[0], v_last = _v_angles[_v_angles.size() - 1];
    if (v_first == 90.0f) {
        if (v_last != 180.0f) {
            return false;
        }
    } else if (v_first != 0.0f) {
        return false;
    }

    return true;
}

bool IESFile::process() {
    if (_h_angles.size() == 0 || _v_angles.size() == 0) {
        return false;
    }

    if (type == TYPE_B) {
        if (!process_type_b()) {
            return false;
        }
    } else {
        assert(type == TYPE_C);
        if (!process_type_c()) {
            return false;
        }
    }

    assert(_v_angles[0] == 0.0f || _v_angles[0] == 90.0f);
    assert(_h_angles[0] == 0.0f);
    assert(_h_angles[_h_angles.size() - 1] == 360.0f);

    /* Convert from deg to rad. */
    for (int i = 0; i < _v_angles.size(); i++) {
        _v_angles[i] *= Pi / 180.f;
    }
    for (int i = 0; i < _h_angles.size(); i++) {
        _h_angles[i] *= Pi / 180.f;
    }

    return true;
}

IESFile::~IESFile() {
    clear();
}

}// namespace vision