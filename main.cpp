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
#include <unordered_map>
#include <cstdlib>

using namespace std;

string current_lang;

string get_locale() {
    const char* lc_all = getenv("LC_ALL");
    if (lc_all && lc_all[0]) return string(lc_all);
    const char* lang = getenv("LANG");
    if (lang && lang[0]) return string(lang);
    return "en";
}

bool starts_with(const string& s, const string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

string detect_lang(const string& locale) {
    if (starts_with(locale, "zh")) return "zh";
    return "en";
}

struct Strings {
    unordered_map<string, unordered_map<string, string>> m;
    Strings() {
        m["en"]["usage"] = "Usage: headtail [OPTIONS]...";
        m["en"]["options"] = "Options:";
        m["en"]["opt_n"] = "  -n [NUM]               Line mode (default), display NUM lines at head/tail.";
        m["en"]["opt_c"] = "  -c [NUM]               Byte/Char mode, display NUM bytes at head/tail.";
        m["en"]["opt_head"] = "  -h, --head NUM         Specify the number of lines/bytes for the head.";
        m["en"]["opt_tail"] = "  -t, --tail NUM         Specify the number of lines/bytes for the tail.";
        m["en"]["opt_input"] = "  -i, --input FILE       Read from FILE instead of standard input.";
        m["en"]["opt_show_size"] = "  -s, --show-size        Show omitted size with auto-scaling unit (KiB, MiB, etc).";
        m["en"]["opt_byte"] = "  --byte                 Force showing omitted size in raw bytes.";
        m["en"]["opt_help"] = "  --help                 Show this help message.";
        m["en"]["err_invalid_arg"] = "Error: Invalid argument \"{1}\" for {0}";
        m["en"]["err_mode_duplicated"] = "Error: Mode duplicated.";
        m["en"]["err_missing_arg"] = "Error: Missing argument for {0}";
        m["en"]["err_is_folder"] = "Error: \"{0}\" is a folder";
        m["en"]["err_permission"] = "Error: permission denied.";
        m["en"]["err_unknown_arg"] = "Error: Unknown argument {0}";
        m["en"]["err_memory"] = "Error: Memory limit exceeded. Tail size too large for current RAM.";
        m["en"]["omit_lines"] = "... ({0} lines omitted) ...";
        m["en"]["omit_bytes"] = "... ({0} chars omitted) ...";
        m["en"]["omit_size"] = "... ({1} bytes has been omitted) ...";
        m["en"]["omit_size_unit"] = "... ({1} {2} has been omitted) ...";
        m["en"]["units_bytes"] = "bytes";
        m["en"]["units_kib"] = "KiB";
        m["en"]["units_mib"] = "MiB";
        m["en"]["units_gib"] = "GiB";
        m["en"]["units_tib"] = "TiB";

        m["zh"]["usage"] = "用法: headtail [选项]...";
        m["zh"]["options"] = "选项:";
        m["zh"]["opt_n"] = "  -n [NUM]               行模式（默认），显示头/尾各 NUM 行。";
        m["zh"]["opt_c"] = "  -c [NUM]               字节模式，显示头/尾各 NUM 字节。";
        m["zh"]["opt_head"] = "  -h, --head NUM         指定头部的行/字节数。";
        m["zh"]["opt_tail"] = "  -t, --tail NUM         指定尾部的行/字节数。";
        m["zh"]["opt_input"] = "  -i, --input 文件       从指定文件读取而非标准输入。";
        m["zh"]["opt_show_size"] = "  -s, --show-size        显示省略大小（自动单位 KiB, MiB 等）。";
        m["zh"]["opt_byte"] = "  --byte                 强制以原始字节显示省略大小。";
        m["zh"]["opt_help"] = "  --help                 显示此帮助信息。";
        m["zh"]["err_invalid_arg"] = "错误: 参数 \"{1}\" 对 {0} 无效";
        m["zh"]["err_mode_duplicated"] = "错误: 模式重复指定。";
        m["zh"]["err_missing_arg"] = "错误: {0} 缺少参数";
        m["zh"]["err_is_folder"] = "错误: \"{0}\" 是一个文件夹";
        m["zh"]["err_permission"] = "错误: 权限不足。";
        m["zh"]["err_unknown_arg"] = "错误: 未知的参数 {0}";
        m["zh"]["err_memory"] = "错误: 内存限制超出。当前 RAM 不足以支持如此大的尾部大小。";
        m["zh"]["omit_lines"] = "... ({0} 行已省略) ...";
        m["zh"]["omit_bytes"] = "... ({0} 字符已省略) ...";
        m["zh"]["omit_size"] = "... ({1} 字节已省略) ...";
        m["zh"]["omit_size_unit"] = "... ({1} {2} 已省略) ...";
        m["zh"]["units_bytes"] = "字节";
        m["zh"]["units_kib"] = "KiB";
        m["zh"]["units_mib"] = "MiB";
        m["zh"]["units_gib"] = "GiB";
        m["zh"]["units_tib"] = "TiB";
    }
    string get(const string& lang, const string& key) {
        auto lit = m.find(lang);
        if (lit == m.end()) lit = m.find("en");
        auto kit = lit->second.find(key);
        if (kit == lit->second.end()) return key;
        return kit->second;
    }
    string get(const string& lang, const string& key, const string& a0) {
        string s = get(lang, key);
        size_t p;
        while ((p = s.find("{0}")) != string::npos) s.replace(p, 3, a0);
        while ((p = s.find("{1}")) != string::npos) s.replace(p, 3, a0);
        return s;
    }
    string get(const string& lang, const string& key, const string& a0, const string& a1) {
        string s = get(lang, key);
        size_t p;
        while ((p = s.find("{0}")) != string::npos) s.replace(p, 3, a0);
        while ((p = s.find("{1}")) != string::npos) s.replace(p, 3, a1);
        return s;
    }
    string get(const string& lang, const string& key, const string& a0, const string& a1, const string& a2) {
        string s = get(lang, key);
        size_t p;
        while ((p = s.find("{0}")) != string::npos) s.replace(p, 3, a0);
        while ((p = s.find("{1}")) != string::npos) s.replace(p, 3, a1);
        while ((p = s.find("{2}")) != string::npos) s.replace(p, 3, a2);
        return s;
    }
} strings;

bool is_known_flag(const string& s) {
    return s == "-h" || s == "--head" || s == "-t" || s == "--tail" ||
           s == "-i" || s == "--input" || s == "-c" || s == "-n" ||
           s == "-s" || s == "--show-size" || s == "--byte" || s == "--help";
}

void err(const string& key) {
    cerr << strings.get(current_lang, key) << "\n";
    exit(1);
}
void err(const string& key, const string& a0) {
    cerr << strings.get(current_lang, key, a0) << "\n";
    exit(1);
}
void err(const string& key, const string& a0, const string& a1) {
    cerr << strings.get(current_lang, key, a0, a1) << "\n";
    exit(1);
}

uint64_t parse_num(const string& val, const string& arg_name) {
    if (!val.empty() && val[0] == '-') {
        err("err_invalid_arg", arg_name, val);
    }
    try {
        double d = std::stod(val);
        if (d < 0) {
            err("err_invalid_arg", arg_name, val);
        }
        if (d >= (double)ULLONG_MAX) return ULLONG_MAX;
        return static_cast<uint64_t>(d);
    } catch (...) {
        err("err_invalid_arg", arg_name, val);
    }
}

void print_omitted_size(uint64_t bytes, bool force_byte) {
    if (force_byte) {
        cout << "\n" << strings.get(current_lang, "omit_size", "", to_string(bytes)) << "\n";
        return;
    }
    const char* unit_keys[] = {"units_bytes", "units_kib", "units_mib", "units_gib", "units_tib"};
    int unit_idx = 0;
    double size = bytes;
    while (size >= 1024.0 && unit_idx < 4) {
        size /= 1024.0;
        unit_idx++;
    }
    if (unit_idx == 0) {
        cout << "\n" << strings.get(current_lang, "omit_size", "", to_string(bytes)) << "\n";
    } else {
        cout << "\n" << strings.get(current_lang, "omit_size_unit", "", to_string(bytes), strings.get(current_lang, unit_keys[unit_idx])) << "\n";
    }
}

class ByteRingBuffer {
    std::string buf;
    uint64_t capacity;
    uint64_t start = 0;
    bool full = false;
public:
    ByteRingBuffer(uint64_t cap) : capacity(cap) {}

    void push(const char* data, size_t len) {
        if (capacity == 0) return;
        try {
            if (!full) {
                uint64_t space = capacity - buf.size();
                if (len <= space) {
                    buf.append(data, len);
                    if (buf.size() == capacity) full = true;
                    return;
                } else {
                    buf.append(data, space);
                    full = true;
                    data += space;
                    len -= space;
                }
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
        } catch (const std::bad_alloc&) {
            cerr << strings.get(current_lang, "err_memory") << "\n";
            exit(1);
        }
    }

    void print() const {
        if (!full) {
            std::cout.write(buf.data(), buf.size());
        } else {
            std::cout.write(buf.data() + start, capacity - start);
            std::cout.write(buf.data(), start);
        }
    }

    uint64_t size() const {
        return full ? capacity : buf.size();
    }

    char get_last_char() const {
        if (size() == 0) return '\n';
        if (!full) return buf.back();
        uint64_t last_idx = (start == 0) ? capacity - 1 : start - 1;
        return buf[last_idx];
    }
};

void print_help() {
    cout << strings.get(current_lang, "usage") << "\n"
         << strings.get(current_lang, "options") << "\n"
         << strings.get(current_lang, "opt_n") << "\n"
         << strings.get(current_lang, "opt_c") << "\n"
         << strings.get(current_lang, "opt_head") << "\n"
         << strings.get(current_lang, "opt_tail") << "\n"
         << strings.get(current_lang, "opt_input") << "\n"
         << strings.get(current_lang, "opt_show_size") << "\n"
         << strings.get(current_lang, "opt_byte") << "\n"
         << strings.get(current_lang, "opt_help") << "\n";
}

int main(int argc, char* argv[]) {
    current_lang = detect_lang(get_locale());

    bool mode_lines = false;
    bool mode_chars = false;
    uint64_t head_count = 10;
    uint64_t tail_count = 10;
    bool show_size = false;
    bool force_byte = false;
    string input_file = "";

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-n" || arg == "-c") {
            if (arg == "-n") {
                if (mode_chars) err("err_mode_duplicated");
                mode_lines = true;
            } else {
                if (mode_lines) err("err_mode_duplicated");
                mode_chars = true;
            }
            if (i + 1 < argc) {
                string next_arg = argv[i+1];
                if (!is_known_flag(next_arg)) {
                    i++;
                    uint64_t val = parse_num(next_arg, arg);
                    head_count = val;
                    tail_count = val;
                }
            }
        } else if (arg == "-h" || arg == "--head") {
            if (i + 1 >= argc) err("err_missing_arg", arg);
            string next_arg = argv[++i];
            if (is_known_flag(next_arg)) {
                err("err_invalid_arg", arg, next_arg);
            }
            head_count = parse_num(next_arg, arg);
        } else if (arg == "-t" || arg == "--tail") {
            if (i + 1 >= argc) err("err_missing_arg", arg);
            string next_arg = argv[++i];
            if (is_known_flag(next_arg)) {
                err("err_invalid_arg", arg, next_arg);
            }
            tail_count = parse_num(next_arg, arg);
        } else if (arg == "-i" || arg == "--input") {
            if (i + 1 >= argc) err("err_missing_arg", arg);
            input_file = argv[++i];
        } else if (arg == "-s" || arg == "--show-size") {
            show_size = true;
        } else if (arg == "--byte") {
            force_byte = true;
            show_size = true;
        } else if (arg == "--help") {
            print_help();
            return 0;
        } else {
            err("err_unknown_arg", arg);
        }
    }

    if (!mode_lines && !mode_chars) {
        mode_lines = true;
    }

    std::istream* in = &std::cin;
    std::ifstream file_in;
    if (!input_file.empty()) {
        std::error_code ec;
        if (std::filesystem::is_directory(input_file, ec)) {
            err("err_is_folder", input_file);
        }
        file_in.open(input_file, std::ios::binary);
        if (!file_in) {
            err("err_permission");
        }
        in = &file_in;
    }

    uint64_t total_bytes = 0;
    uint64_t head_printed = 0;
    char chunk[8192];
    char last_printed_char = '\n';

    if (mode_lines) {
        uint64_t total_lines = 0;
        uint64_t head_bytes_printed = 0;
        uint64_t tail_bytes_stored = 0;
        std::deque<std::string> tail_buffer;
        std::string current_line;

        auto process_line = [&](const std::string& line) {
            total_lines++;
            uint64_t line_bytes = line.size();
            if (head_printed < head_count) {
                cout << line;
                if (!line.empty()) last_printed_char = line.back();
                head_printed++;
                head_bytes_printed += line_bytes;
            } else {
                tail_buffer.push_back(line);
                tail_bytes_stored += line_bytes;
                if (tail_buffer.size() > tail_count) {
                    tail_bytes_stored -= tail_buffer.front().size();
                    tail_buffer.pop_front();
                }
            }
        };

        while (in->read(chunk, sizeof(chunk)) || in->gcount() > 0) {
            size_t len = in->gcount();
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

        if (total_lines > head_count + tail_count) {
            if (show_size) {
                uint64_t omitted_bytes = total_bytes - head_bytes_printed - tail_bytes_stored;
                print_omitted_size(omitted_bytes, force_byte);
            } else {
                cout << "\n" << strings.get(current_lang, "omit_lines", to_string(total_lines - head_printed - tail_buffer.size())) << "\n";
            }
            last_printed_char = '\n';
        }

        for (const auto& l : tail_buffer) {
            cout << l;
            if (!l.empty()) last_printed_char = l.back();
        }

        if (last_printed_char != '\n') cout << '\n';

        if (head_count + tail_count >= total_lines) return 1;
        return 0;

    } else {
        ByteRingBuffer tail_buf(tail_count);

        while (in->read(chunk, sizeof(chunk)) || in->gcount() > 0) {
            size_t len = in->gcount();
            total_bytes += len;
            size_t chunk_pos = 0;

            if (head_printed < head_count) {
                uint64_t need = head_count - head_printed;
                uint64_t take = std::min((uint64_t)len, need);
                std::cout.write(chunk, take);
                if (take > 0) last_printed_char = chunk[chunk_pos + take - 1];
                head_printed += take;
                chunk_pos += take;
            }

            if (chunk_pos < len) {
                tail_buf.push(chunk + chunk_pos, len - chunk_pos);
            }
        }

        if (total_bytes > head_count + tail_count) {
            if (show_size) {
                uint64_t omitted_bytes = total_bytes - head_printed - tail_buf.size();
                print_omitted_size(omitted_bytes, force_byte);
            } else {
                cout << "\n" << strings.get(current_lang, "omit_bytes", to_string(total_bytes - head_printed - tail_buf.size())) << "\n";
            }
            last_printed_char = '\n';
        }

        tail_buf.print();
        if (tail_buf.size() > 0) {
            last_printed_char = tail_buf.get_last_char();
        }

        if (last_printed_char != '\n') cout << '\n';

        if (head_count + tail_count >= total_bytes) return 1;
        return 0;
    }
}