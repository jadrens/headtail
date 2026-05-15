#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <climits>
#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include "src/i18n/i18n.h"

using namespace std;

static string g_lang;

enum class Mode { NONE, LINES, CHARS };

struct AppState {
    Mode mode = Mode::NONE;
    uint64_t head_count = 10;
    uint64_t tail_count = 10;
    bool show_size = false;
    bool force_byte = false;
    string input_file;
};

static void err_exit(const char* key) {
    cerr << get_string(key, g_lang) << "\n";
    exit(1);
}

static void err_exit(const char* key, const char* a0) {
    string s = get_string(key, g_lang);
    size_t p;
    while ((p = s.find("{0}")) != string::npos) s.replace(p, 3, a0);
    cerr << s << "\n";
    exit(1);
}

static void err_exit(const char* key, const char* a0, const char* a1) {
    string s = get_string(key, g_lang);
    size_t p;
    while ((p = s.find("{0}")) != string::npos) s.replace(p, 3, a0);
    while ((p = s.find("{1}")) != string::npos) s.replace(p, 3, a1);
    cerr << s << "\n";
    exit(1);
}

static uint64_t parse_num(const string& val, const string& arg_name) {
    if (!val.empty() && val[0] == '-') {
        err_exit("err_invalid_arg", arg_name.c_str(), val.c_str());
    }
    try {
        double d = stod(val);
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

static void print_omitted_size(uint64_t bytes, bool force_byte) {
    const char* unit_keys[] = {"units_bytes", "units_kib", "units_mib", "units_gib", "units_tib"};
    int unit_idx = 0;
    double size = bytes;
    while (size >= 1024.0 && unit_idx < 4) {
        size /= 1024.0;
        unit_idx++;
    }

    string out = "\n";
    if (force_byte || unit_idx == 0) {
        out += get_string("omit_size", g_lang);
    } else {
        out += get_string("omit_size_unit", g_lang);
    }
    out += "\n";

    size_t p;
    while ((p = out.find("{1}")) != string::npos) out.replace(p, 3, to_string(bytes));
    while ((p = out.find("{2}")) != string::npos) out.replace(p, 3, get_string(unit_keys[unit_idx], g_lang));
    cout << out;
}

class ByteRingBuffer {
    string buf;
    uint64_t capacity;
    uint64_t start = 0;
    bool full = false;
public:
    explicit ByteRingBuffer(uint64_t cap) : capacity(cap) {}


    void push(const char* data, size_t len) {
        if (capacity == 0) return;
        try {
            if (!full) {
                uint64_t space = capacity - buf.size();
                if (len <= space) {
                    buf.append(data, len);
                    if (buf.size() == capacity) full = true;
                    return;
                }
                buf.append(data, space);
                full = true;
                data += space;
                len -= space;
            }
            if (len >= capacity) {
                data += (len - capacity);
                len = capacity;
                buf.assign(data, len);
                start = 0;
            } else {
                uint64_t end_space = capacity - start;
                if (len <= end_space) {
                    buf.replace(start, len, data, len);
                    start = (start + len) % capacity;
                } else {
                    buf.replace(start, end_space, data, end_space);
                    uint64_t rem = len - end_space;
                    buf.replace(0, rem, data + end_space, rem);
                    start = rem;
                }
            }
        } catch (const bad_alloc&) {
            err_exit("err_memory");
        }
    }

    void print() const {
        if (!full) {
            cout.write(buf.data(), buf.size());
        } else {
            cout.write(buf.data() + start, capacity - start);
            cout.write(buf.data(), start);
        }
    }

    uint64_t size() const { return full ? capacity : buf.size(); }

    char get_last_char() const {
        if (size() == 0) return '\n';
        if (!full) return buf.back();
        uint64_t last_idx = (start == 0) ? capacity - 1 : start - 1;
        return buf[last_idx];
    }
};

static void print_help() {
    cout << get_string("usage", g_lang) << "\n"
         << get_string("options", g_lang) << "\n"
         << get_string("opt_n", g_lang) << "\n"
         << get_string("opt_c", g_lang) << "\n"
         << get_string("opt_head", g_lang) << "\n"
         << get_string("opt_tail", g_lang) << "\n"
         << get_string("opt_input", g_lang) << "\n"
         << get_string("opt_show_size", g_lang) << "\n"
         << get_string("opt_byte", g_lang) << "\n"
         << get_string("opt_help", g_lang) << "\n";
}

static bool is_flag(const string& s) {
    return s == "-h" || s == "--head" || s == "-t" || s == "--tail" ||
           s == "-i" || s == "--input" || s == "-c" || s == "-n" ||
           s == "-s" || s == "--show-size" || s == "--byte" || s == "--help";
}

static void parse_args(int argc, char* argv[], AppState& state) {
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg == "-n") {
            if (state.mode == Mode::CHARS) err_exit("err_mode_duplicated");
            state.mode = Mode::LINES;
            if (i + 1 < argc && !is_flag(argv[i + 1])) {
                state.head_count = state.tail_count = parse_num(argv[++i], arg);
            }
        } else if (arg == "-c") {
            if (state.mode == Mode::LINES) err_exit("err_mode_duplicated");
            state.mode = Mode::CHARS;
            if (i + 1 < argc && !is_flag(argv[i + 1])) {
                state.head_count = state.tail_count = parse_num(argv[++i], arg);
            }
        } else if (arg == "-h" || arg == "--head") {
            if (++i >= argc) err_exit("err_missing_arg", arg.c_str());
            if (is_flag(argv[i])) err_exit("err_invalid_arg", arg.c_str(), argv[i]);
            state.head_count = parse_num(argv[i], arg);
        } else if (arg == "-t" || arg == "--tail") {
            if (++i >= argc) err_exit("err_missing_arg", arg.c_str());
            if (is_flag(argv[i])) err_exit("err_invalid_arg", arg.c_str(), argv[i]);
            state.tail_count = parse_num(argv[i], arg);
        } else if (arg == "-i" || arg == "--input") {
            if (++i >= argc) err_exit("err_missing_arg", arg.c_str());
            state.input_file = argv[i];
        } else if (arg == "-s" || arg == "--show-size") {
            state.show_size = true;
        } else if (arg == "--byte") {
            state.force_byte = true;
            state.show_size = true;
        } else if (arg == "--help") {
            print_help();
            exit(0);
        } else if (arg[0] != '-') {
            state.input_file = arg;
        } else {
            err_exit("err_unknown_arg", arg.c_str());
        }
    }

    if (state.mode == Mode::NONE) {
        state.mode = Mode::LINES;
    }
}

static istream* open_input(const string& file) {
    if (file.empty()) return &cin;

    error_code ec;
    if (filesystem::is_directory(file, ec)) {
        err_exit("err_is_folder", file.c_str());
    }
    ifstream* f = new ifstream(file, ios::binary);
    if (!*f) {
        delete f;
        err_exit("err_permission");
    }
    return f;
}

static void process_lines(istream& in, const AppState& state) {
    uint64_t total_bytes = 0;
    uint64_t head_printed = 0;
    uint64_t head_bytes_printed = 0;
    uint64_t tail_bytes_stored = 0;
    deque<string> tail_buffer;
    string current_line;
    char last_char = '\n';
    char chunk[8192];

    auto process_line = [&](const string& line) {
        uint64_t line_bytes = line.size();
        if (head_printed < state.head_count) {
            cout << line;
            if (!line.empty()) last_char = line.back();
            head_printed++;
            head_bytes_printed += line_bytes;
        } else {
            tail_buffer.push_back(line);
            tail_bytes_stored += line_bytes;
            if (tail_buffer.size() > state.tail_count) {
                tail_bytes_stored -= tail_buffer.front().size();
                tail_buffer.pop_front();
            }
        }
    };

    while (in.read(chunk, sizeof(chunk)) || in.gcount() > 0) {
        size_t len = in.gcount();
        total_bytes += len;
        for (size_t i = 0; i < len; i++) {
            current_line += chunk[i];
            if (chunk[i] == '\n') {
                process_line(current_line);
                current_line.clear();
            }
        }
    }
    if (!current_line.empty()) {
        process_line(current_line);
    }

    uint64_t total_lines = head_printed + tail_buffer.size();
    if (total_lines > state.head_count + state.tail_count) {
        if (state.show_size) {
            uint64_t omitted = total_bytes - head_bytes_printed - tail_bytes_stored;
            print_omitted_size(omitted, state.force_byte);
        } else {
            string s = get_string("omit_lines", g_lang);
            size_t p;
            while ((p = s.find("{0}")) != string::npos) {
                s.replace(p, 3, to_string(total_lines - head_printed - tail_buffer.size()));
            }
            cout << "\n" << s << "\n";
        }
        last_char = '\n';
    }

    for (const auto& l : tail_buffer) {
        cout << l;
        if (!l.empty()) last_char = l.back();
    }

    if (last_char != '\n') cout << '\n';
}

static void process_chars(istream& in, const AppState& state) {
    uint64_t total_bytes = 0;
    uint64_t head_printed = 0;
    ByteRingBuffer tail_buf(state.tail_count);
    char last_char = '\n';
    char chunk[8192];

    while (in.read(chunk, sizeof(chunk)) || in.gcount() > 0) {
        size_t len = in.gcount();
        total_bytes += len;
        size_t pos = 0;

        if (head_printed < state.head_count) {
            uint64_t need = state.head_count - head_printed;
            uint64_t take = min((uint64_t)len, need);
            cout.write(chunk, take);
            if (take > 0) last_char = chunk[take - 1];
            head_printed += take;
            pos += take;
        }

        if (pos < len) {
            tail_buf.push(chunk + pos, len - pos);
        }
    }

    if (total_bytes > state.head_count + state.tail_count) {
        if (state.show_size) {
            uint64_t omitted = total_bytes - head_printed - tail_buf.size();
            print_omitted_size(omitted, state.force_byte);
        } else {
            string s = get_string("omit_bytes", g_lang);
            size_t p;
            while ((p = s.find("{0}")) != string::npos) {
                s.replace(p, 3, to_string(total_bytes - head_printed - tail_buf.size()));
            }
            cout << "\n" << s << "\n";
        }
        last_char = '\n';
    }

    tail_buf.print();
    if (tail_buf.size() > 0) last_char = tail_buf.get_last_char();
    if (last_char != '\n') cout << '\n';
}

int main(int argc, char* argv[]) {
    g_lang = detect_lang(get_locale());

    AppState state;
    parse_args(argc, argv, state);

    istream* in = open_input(state.input_file);
    ifstream* file_in = dynamic_cast<ifstream*>(in);
    bool need_delete = file_in != nullptr;

    if (state.mode == Mode::LINES) {
        process_lines(*in, state);
    } else {
        process_chars(*in, state);
    }

    if (need_delete) delete in;
    return 0;
}