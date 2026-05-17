#pragma once
#include <string>
#include <cstdint>
#include <iosfwd>

struct AppState {
    enum class Mode { NONE, LINES, CHARS };
    Mode mode = Mode::NONE;
    uint64_t head_count = 10;
    uint64_t tail_count = 10;
    bool show_size = false;
    bool force_byte = false;
    std::string input_file;
};

AppState& get_state();
void print_help();
bool is_flag(const std::string& s);
void parse_args(int argc, char* argv[]);
std::istream* open_input(const std::string& file);