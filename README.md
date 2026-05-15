# headtail

A combined `head` and `tail` utility for displaying the beginning and end of files or stdin input.

## Build

```bash
cmake -B build
cmake --build build
```

## Install

```bash
cmake --install build
```

## Usage

```bash
# Show first and last 10 lines (default)
headtail input.txt

# Show first/last 20 lines
headtail -n 20 input.txt

# Different head and tail counts
headtail -h 5 -t 15 input.txt

# Byte mode instead of line mode
headtail -c 100 input.txt

# Read from stdin
cat file.txt | headtail -n 10

# Show omitted content size
headtail -s input.txt
```