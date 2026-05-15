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

using namespace std;

bool is_known_flag(const string& s) {
    return s == "-h" || s == "--head" || s == "-t" || s == "--tail" ||
           s == "-i" || s == "--input" || s == "-c" || s == "-n" ||
           s == "-s" || s == "--show-size" || s == "--byte" || s == "--help";
}

uint64_t parse_num(const string& val, const string& arg_name) {
    if (!val.empty() && val[0] == '-') {
        cerr << "Error: Invaild argument \"" << val << "\" for " << arg_name << "\n";
        exit(1);
    }
    try {
        double d = std::stod(val);
        if (d < 0) {
            cerr << "Error: Invaild argument \"" << val << "\" for " << arg_name << "\n";
            exit(1);
        }
        if (d >= (double)ULLONG_MAX) return ULLONG_MAX;
        return static_cast<uint64_t>(d);
    } catch (...) {
        cerr << "Error: Invaild argument \"" << val << "\" for " << arg_name << "\n";
        exit(1);
    }
}

// 重点修改 1：在省略提示的前后都加上了换行符 \n
void print_omitted_size(uint64_t bytes, bool force_byte) {
    if (force_byte) {
        cout << "\n... (" << bytes << " bytes has been omittd) ...\n";
        return;
    }
    const char* units[] = {"bytes", "KiB", "MiB", "GiB", "TiB"};
    int unit_idx = 0;
    double size = bytes;
    while (size >= 1024.0 && unit_idx < 4) {
        size /= 1024.0;
        unit_idx++;
    }
    if (unit_idx == 0) {
        cout << "\n... (" << bytes << " bytes has been omittd) ...\n";
    } else {
        cout << "\n... (" << std::fixed << std::setprecision(2) << size 
             << " " << units[unit_idx] << " has been omittd) ...\n";
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
            cerr << "Error: Memory limit exceeded. Tail size too large for current RAM.\n";
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

    // 新增：获取缓冲区的最后一个字符，用于判断末尾换行
    char get_last_char() const {
        if (size() == 0) return '\n';
        if (!full) return buf.back();
        uint64_t last_idx = (start == 0) ? capacity - 1 : start - 1;
        return buf[last_idx];
    }
};

void print_help() {
    cout << "Usage: headtail [OPTIONS]...\n"
         << "Options:\n"
         << "  -n [NUM]               Line mode (default), display NUM lines at head/tail.\n"
         << "  -c [NUM]               Byte/Char mode, display NUM bytes at head/tail.\n"
         << "  -h, --head NUM         Specify the number of lines/bytes for the head.\n"
         << "  -t, --tail NUM         Specify the number of lines/bytes for the tail.\n"
         << "  -i, --input FILE       Read from FILE instead of standard input.\n"
         << "  -s, --show-size        Show omitted size with auto-scaling unit (KiB, MiB, etc).\n"
         << "  --byte                 Force showing omitted size in raw bytes.\n"
         << "  --help                 Show this help message.\n";
}

int main(int argc, char* argv[]) {
    bool mode_lines = false;
    bool mode_chars = false;
    uint64_t head_count = 10;
    uint64_t tail_count = 10;
    bool head_set = false;
    bool tail_set = false;
    bool show_size = false;
    bool force_byte = false;
    string input_file = "";

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-n" || arg == "-c") {
            if (arg == "-n") {
                if (mode_chars) { cerr << "Error: Mode duplicated.\n"; exit(1); }
                mode_lines = true;
            } else {
                if (mode_lines) { cerr << "Error: Mode duplicated.\n"; exit(1); }
                mode_chars = true;
            }
            if (i + 1 < argc) {
                string next_arg = argv[i+1];
                if (!is_known_flag(next_arg)) {
                    i++;
                    uint64_t val = parse_num(next_arg, arg);
                    head_count = val;
                    tail_count = val;
                    head_set = true;
                    tail_set = true;
                }
            }
        } else if (arg == "-h" || arg == "--head") {
            if (i + 1 >= argc) { cerr << "Error: Missing argument for " << arg << "\n"; exit(1); }
            string next_arg = argv[++i];
            if (is_known_flag(next_arg)) {
                cerr << "Error: Invaild argument \"" << next_arg << "\" for " << arg << "\n"; exit(1);
            }
            head_count = parse_num(next_arg, arg);
            head_set = true;
        } else if (arg == "-t" || arg == "--tail") {
            if (i + 1 >= argc) { cerr << "Error: Missing argument for " << arg << "\n"; exit(1); }
            string next_arg = argv[++i];
            if (is_known_flag(next_arg)) {
                cerr << "Error: Invaild argument \"" << next_arg << "\" for " << arg << "\n"; exit(1);
            }
            tail_count = parse_num(next_arg, arg);
            tail_set = true;
        } else if (arg == "-i" || arg == "--input") {
            if (i + 1 >= argc) { cerr << "Error: Missing argument for " << arg << "\n"; exit(1); }
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
            cerr << "Error: Unknown argument " << arg << "\n";
            exit(1);
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
            cerr << "Error: \"" << input_file << "\" is a folder\n";
            exit(1);
        }
        file_in.open(input_file, std::ios::binary);
        if (!file_in) {
            cerr << "Error: permission denied.\n";
            exit(1);
        }
        in = &file_in;
    }

    uint64_t total_bytes = 0;
    uint64_t head_printed = 0;
    char chunk[8192];
    char last_printed_char = '\n'; // 新增：全局追踪最后打印的字符

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
                // 重点修改 2：在非 size 模式下的提示前后也加上 \n
                cout << "\n... (" << (total_lines - head_printed - tail_buffer.size()) << " lines omitted) ...\n";
            }
            last_printed_char = '\n'; // 提示语以 \n 结尾
        }

        for (const auto& l : tail_buffer) {
            cout << l;
            if (!l.empty()) last_printed_char = l.back();
        }

        // 重点修改 3：末尾缺失 \n 时自动补齐
        if (last_printed_char != '\n') cout << '\n';

        if (head_count + tail_count >= total_lines) return 1;
        return 0;

    } else {
        // CHAR MODE
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
                // 重点修改 2：在非 size 模式下的提示前后也加上 \n
                cout << "\n... (" << (total_bytes - head_printed - tail_buf.size()) << " chars omitted) ...\n";
            }
            last_printed_char = '\n';
        }

        tail_buf.print();
        if (tail_buf.size() > 0) {
            last_printed_char = tail_buf.get_last_char();
        }

        // 重点修改 3：末尾缺失 \n 时自动补齐
        if (last_printed_char != '\n') cout << '\n';

        if (head_count + tail_count >= total_bytes) return 1;
        return 0;
    }
}