#include "err.h"
#include "i18n.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <climits>
#include <cmath>

extern std::string g_lang;

void err_exit(const char* key) {
    std::cerr << get_string(key, g_lang) << "\n";
    std::exit(1);
}

void err_exit(const char* key, const char* a0) {
    std::string s = get_string(key, g_lang);
    size_t p;
    while ((p = s.find("{0}")) != std::string::npos) s.replace(p, 3, a0);
    std::cerr << s << "\n";
    std::exit(1);
}

void err_exit(const char* key, const char* a0, const char* a1) {
    std::string s = get_string(key, g_lang);
    size_t p;
    while ((p = s.find("{0}")) != std::string::npos) s.replace(p, 3, a0);
    while ((p = s.find("{1}")) != std::string::npos) s.replace(p, 3, a1);
    std::cerr << s << "\n";
    std::exit(1);
}

uint64_t do_parse_num(const std::string& val, const std::string& arg_name) {
    if (!val.empty() && val[0] == '-') {
        err_exit("err_invalid_arg", arg_name.c_str(), val.c_str());
    }
    try {
        double d = std::stod(val);
        if (d < 0) {
            err_exit("err_invalid_arg", arg_name.c_str(), val.c_str());
        }
        if (d >= (double)ULLONG_MAX) return ULLONG_MAX;
        return static_cast<uint64_t>(d);
    } catch (...) {
        err_exit("err_invalid_arg", arg_name.c_str(), val.c_str());
    }
    return 0;
}

void print_omitted_size(uint64_t bytes, bool force_byte) {
    const char* unit_keys[] = {"units_bytes", "units_kib", "units_mib", "units_gib", "units_tib"};
    int unit_idx = 0;
    double size = bytes;
    while (size >= 1024.0 && unit_idx < 4) {
        size /= 1024.0;
        unit_idx++;
    }

    std::string out = "\n";
    if (force_byte || unit_idx == 0) {
        out += get_string("omit_size", g_lang);
    } else {
        out += get_string("omit_size_unit", g_lang);
    }
    out += "\n";

    size_t p;
    while ((p = out.find("{1}")) != std::string::npos) out.replace(p, 3, std::to_string(bytes));
    while ((p = out.find("{2}")) != std::string::npos) out.replace(p, 3, get_string(unit_keys[unit_idx], g_lang));
    std::cout << out;
}