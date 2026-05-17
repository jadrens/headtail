#pragma once
#include <cstdint>
#include <string>
#include <cstddef>
#include <iosfwd>

class ByteRingBuffer {
    std::string buf;
    uint64_t capacity;
    uint64_t start = 0;
    bool full = false;
public:
    explicit ByteRingBuffer(uint64_t cap);
    void push(const char* data, size_t len);
    void print() const;
    uint64_t size() const;
    char get_last_char() const;
};

void process_lines(std::istream& in);
void process_chars(std::istream& in);