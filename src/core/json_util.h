//
// Created by Zero on 2023/6/16.
//

#pragma once

#include "core/stl.h"
#include "core/string_util.h"

namespace vision {
namespace detail {
[[nodiscard]] inline std::string remove_cxx_comment(std::string source) {
    if (source.size() < 2) {
        return std::move(source);
    }

    const char *p0 = source.data();
    const char *p = p0;
    const char *p2 = p + 1;
    const char *pend = p + source.size();

    bool in_quote = false;
    bool in_sline_comment = false;
    bool in_mline_comment = false;
    bool all_whitespace = true;
    const char *pcontent = p;

    std::ostringstream ostrm;

    for (; p2 < pend; ++p, ++p2) {

        if (in_quote) {
            if (*p == '"')
                in_quote = false;
            continue;
        }

        if (in_sline_comment) {
            if (*p == '\n') {
                in_sline_comment = false;
                pcontent = p + (int)all_whitespace;
            } else if (*p == '\r' && *p2 == '\n') {
                in_sline_comment = false;
                pcontent = p + ((int)all_whitespace << 1);
                p = p2;
                ++p2;
            }
        } else {
            if (in_mline_comment) {
                if (*p == '*' && *p2 == '/') {
                    in_mline_comment = false;
                    pcontent = p + 2;
                    p = p2;
                    ++p2;
                }
            } else {
                // !in_quote && !in_sline_comment && !in_mline_comment
                if (*p == '"') {
                    in_quote = true;
                    all_whitespace = false;
                } else if (*p == '/') {
                    if (*p2 == '*') {
                        in_mline_comment = true;
                        ostrm.write(pcontent, p - pcontent);
                        p = p2;
                        ++p2;
                    } else if (*p2 == '/') {
                        in_sline_comment = true;
                        ostrm.write(pcontent, p - pcontent);
                        p = p2;
                        ++p2;
                    } else
                        all_whitespace = false;
                } else if (*p == '\n') {
                    all_whitespace = true;
                } else if (*p == '\r' && *p2 == '\n') {
                    all_whitespace = true;
                    p = p2;
                    ++p2;
                } else if (all_whitespace && *p != ' ')
                    all_whitespace = false;
            }
        }
    }

    if (!in_sline_comment && pcontent != pend) {
        if (pcontent == p0)
            return std::move(source);
        else
            ostrm.write(pcontent, pend - pcontent);
    }

    return ostrm.str();
}
}// namespace detail

[[nodiscard]] inline DataWrap create_json_from_file(const fs::path &fn) {
    std::ifstream fst;
    fst.open(fn.c_str());
    std::stringstream buffer;
    buffer << fst.rdbuf();
    std::string str = buffer.str();
    str = detail::remove_cxx_comment(std::move(str));
    fst.close();
    if (fn.extension() == ".bson") {
        return DataWrap::from_bson(str);
    } else {
        return DataWrap::parse(str);
    }
}

}// namespace vision