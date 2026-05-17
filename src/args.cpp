#include "args.h"
#include "err.h"
#include "process.h"
#include "i18n.h"
#include "utils/cast.cpp"
#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <queue>

extern std::string g_lang;
AppState g_state;

AppState& get_state() {
    return g_state;
}

void print_help() {
    std::cout << get_string("usage", g_lang) << "\n"
         << get_string("options", g_lang) << "\n"
         << get_string("opt_n", g_lang) << "\n"
         << get_string("opt_c", g_lang) << "\n"
         << get_string("opt_head", g_lang) << "\n"
         << get_string("opt_tail", g_lang) << "\n"
         << get_string("opt_input", g_lang) << "\n"
         << get_string("opt_show_size", g_lang) << "\n"
         << get_string("opt_byte", g_lang) << "\n"
         << get_string("opt_keyword", g_lang) << "\n"
         << get_string("opt_keyword_case", g_lang) << "\n"
         << get_string("opt_help", g_lang) << "\n";
}

static const std::unordered_set<std::string> flag_set = {
    "-h", "--head", "-t", "--tail", "-i", "--input",
    "-c", "-n", "-s", "--show-size", "--byte", "--help", "-k", "--keyword", "-K"
};

bool is_flag(const std::string& s) {
    return flag_set.contains(s);
}

static void require_arg(int& i, int argc, std::deque<std::string>& args, const char* flag) {
    if (++i >= argc || is_flag(args[i])) err_exit("err_missing_arg", flag);
    // Note: removed erroneous mode duplication check - input file can be used with any mode
}

uint64_t do_parse_num(const std::string& val, const std::string& arg_name);

void parse_args(int argc, char* argv[]) {
    using Handler = std::function<void(int&, int, std::deque<std::string>&)>;
    static const std::map<std::string, Handler> handlers = {
        {"-n", [](int& i, int argc, std::deque<std::string>& args) {
            g_state.mode = AppState::Mode::LINES;
            if (i + 1 < argc && !is_flag(args[i + 1])) {
                g_state.head_count = g_state.tail_count = do_parse_num(args[++i], "-n");
            }
        }},
        {"-c", [](int& i, int argc, std::deque<std::string>& args) {
            g_state.mode = AppState::Mode::CHARS;
            if (i + 1 < argc && !is_flag(args[i + 1])) {
                g_state.head_count = g_state.tail_count = do_parse_num(args[++i], "-c");
            }
        }},
        {"-h", [](int& i, int argc, std::deque<std::string>& args) {
            require_arg(i, argc, args, "-h");
            g_state.head_count = do_parse_num(args[i], "-h");
        }},
        {"--head", [](int& i, int argc, std::deque<std::string>& args) {
            require_arg(i, argc, args, "--head");
            g_state.head_count = do_parse_num(args[i], "--head");
        }},
        {"-t", [](int& i, int argc, std::deque<std::string>& args) {
            require_arg(i, argc, args, "-t");
            g_state.tail_count = do_parse_num(args[i], "-t");
        }},
        {"--tail", [](int& i, int argc, std::deque<std::string>& args) {
            require_arg(i, argc, args, "--tail");
            g_state.tail_count = do_parse_num(args[i], "--tail");
        }},
        {"-i", [](int& i, int argc, std::deque<std::string>& args) {
            require_arg(i, argc, args, "-i");
            g_state.input_file = args[i];
        }},
        {"--input", [](int& i, int argc, std::deque<std::string>& args) {
            require_arg(i, argc, args, "--input");
            g_state.input_file = args[i];
        }},
        {"-s", [](int&, int, std::deque<std::string>&) { g_state.show_size = true; }},
        {"--show-size", [](int&, int, std::deque<std::string>&) { g_state.show_size = true; }},
        {"--byte", [](int&, int, std::deque<std::string>&) { g_state.force_byte = true; g_state.show_size = true; }},
        {"-k", [](int& i, int argc, std::deque<std::string>& args) {
            require_arg(i, argc, args, "-k");
            std::string kw = args[i];
            size_t start = 0;
            while (start < kw.size()) {
                size_t comma = kw.find(',', start);
                std::string word = kw.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
                if (!word.empty()) {
                    // convert to lowercase for case-insensitive matching
                    for (auto& c : word) c = std::tolower(static_cast<unsigned char>(c));
                    g_state.keywords.push_back(word);
                }
                if (comma == std::string::npos) break;
                start = comma + 1;
            }
        }},
        {"--keyword", [](int& i, int argc, std::deque<std::string>& args) {
            require_arg(i, argc, args, "--keyword");
            std::string kw = args[i];
            size_t start = 0;
            while (start < kw.size()) {
                size_t comma = kw.find(',', start);
                std::string word = kw.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
                if (!word.empty()) {
                    for (auto& c : word) c = std::tolower(static_cast<unsigned char>(c));
                    g_state.keywords.push_back(word);
                }
                if (comma == std::string::npos) break;
                start = comma + 1;
            }
        }},
        {"-K", [](int& i, int argc, std::deque<std::string>& args) {
            require_arg(i, argc, args, "-K");
            std::string kw = args[i];
            size_t start = 0;
            while (start < kw.size()) {
                size_t comma = kw.find(',', start);
                std::string word = kw.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
                if (!word.empty()) g_state.keywords_case.push_back(word);
                if (comma == std::string::npos) break;
                start = comma + 1;
            }
        }},
        {"--help", [](int&, int, std::deque<std::string>&) { print_help(); std::exit(0); }},
    };

    std::deque<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }
    if (!args.empty() && args.front()[0] != '-') {
        // fast mode
        try {
            size_t first_error;
            uint64_t head = safe_double_to_uint64(std::stod(args[0], &first_error));
            if (first_error != args[0].length()) throw std::invalid_argument("Not a number");
            g_state.head_count = head;
            if (argc > 2) {
                try {
                    uint64_t tail = safe_double_to_uint64(std::stod(args[1], &first_error));
                    if (first_error != args[1].length()) throw std::invalid_argument("Not a number");
                    g_state.tail_count = tail;
                    args.pop_front();
                } catch (...) {
                    g_state.tail_count = g_state.head_count;
                }
            } else {
                g_state.tail_count = g_state.head_count;
            }
            args.pop_front();
        } catch (...) {
            if (!args.empty()) {
                g_state.input_file = args.front();
                args.pop_front();
            }
        }
    }

    for (int i = 0; i < static_cast<int>(args.size()); i++) {
        auto it = handlers.find(args[i]);
        if (it != handlers.end()) {
            it->second(i, static_cast<int>(args.size()), args);
        } else {
            err_exit("err_unknown_arg", args[i].c_str());
        }
    }


    // default mode
    if (g_state.mode == AppState::Mode::NONE) {
        g_state.mode = AppState::Mode::LINES;
    }

    // default when no numbers specified
    if (g_state.head_count == 0 && g_state.tail_count == 0) {
        g_state.head_count = g_state.tail_count = 3;
    }
}

std::istream* open_input(const std::string& file) {
    if (file.empty()) return &std::cin;

    std::error_code ec;
    if (std::filesystem::is_directory(file, ec)) {
        err_exit("err_is_folder", file.c_str());
    }
    std::ifstream* f = new std::ifstream(file, std::ios::binary);
    if (!*f) {
        delete f;
        err_exit("err_permission");
    }
    return f;
}