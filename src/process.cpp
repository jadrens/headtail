#include "process.h"
#include "args.h"
#include "err.h"
#include "i18n.h"
#include <iostream>
#include <string>
#include <deque>
#include <fstream>
#include <cstdlib>
#include <climits>

extern std::string g_lang;

ByteRingBuffer::ByteRingBuffer(uint64_t cap) : capacity(cap) {}

void ByteRingBuffer::push(const char* data, size_t len) {
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
    } catch (const std::bad_alloc&) {
        err_exit("err_memory");
    }
}

void ByteRingBuffer::print() const {
    if (!full) {
        std::cout.write(buf.data(), buf.size());
    } else {
        std::cout.write(buf.data() + start, capacity - start);
        std::cout.write(buf.data(), start);
    }
}

uint64_t ByteRingBuffer::size() const {
    return full ? capacity : buf.size();
}

char ByteRingBuffer::get_last_char() const {
    if (size() == 0) return '\n';
    if (!full) return buf.back();
    uint64_t last_idx = (start == 0) ? capacity - 1 : start - 1;
    return buf[last_idx];
}

void process_lines(std::istream& in) {
    AppState& state = get_state();
    uint64_t total_bytes = 0;
    uint64_t head_printed = 0;
    uint64_t head_bytes_printed = 0;
    uint64_t tail_bytes_stored = 0;
    std::deque<std::string> tail_buffer;
    std::string current_line;
    char last_char = '\n';
    char chunk[8192];

    auto process_line = [&](const std::string& line) {
        uint64_t line_bytes = line.size();
        if (head_printed < state.head_count) {
            std::cout << line;
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
            std::string s = get_string("omit_lines", g_lang);
            size_t p;
            while ((p = s.find("{0}")) != std::string::npos) {
                s.replace(p, 3, std::to_string(total_lines - head_printed - tail_buffer.size()));
            }
            std::cout << "\n" << s << "\n";
        }
        last_char = '\n';
    }

    for (const auto& l : tail_buffer) {
        std::cout << l;
        if (!l.empty()) last_char = l.back();
    }

    if (last_char != '\n') std::cout << '\n';
}

void process_chars(std::istream& in) {
    AppState& state = get_state();
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
            uint64_t take = std::min((uint64_t)len, need);
            std::cout.write(chunk, take);
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
            std::string s = get_string("omit_bytes", g_lang);
            size_t p;
            while ((p = s.find("{0}")) != std::string::npos) {
                s.replace(p, 3, std::to_string(total_bytes - head_printed - tail_buf.size()));
            }
            std::cout << "\n" << s << "\n";
        }
        last_char = '\n';
    }

    tail_buf.print();
    if (tail_buf.size() > 0) last_char = tail_buf.get_last_char();
    if (last_char != '\n') std::cout << '\n';
}