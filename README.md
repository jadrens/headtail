# headtail

A combined `head` and `tail` utility for displaying the beginning and end of files or stdin input.

## Install

### Arch Linux (AUR)
```bash
yay -Sy headtail-git
# or
paru -S headtail-git
```

### From source
```bash
cmake -B build
cmake --build build
sudo cmake --install build
```

## Usage

```bash
# Show first and last 10 lines (default)
headtail input.txt

# Fast mode: first arg is head count, second (optional) is tail count
headtail 10 -i input.txt        # Show first and last 10 lines
headtail 10 20 -i input.txt     # Show first 10 and last 20 lines

# Different head and tail counts
headtail -h 5 -t 15 input.txt

# Byte mode instead of line mode
headtail -c 100 -i input.txt

# Read from stdin
cat file.txt | headtail -n 10

# Show omitted content size
headtail -s -i input.txt

# Keyword matching (case-sensitive), trigger immediate output when keyword appears in tail buffer
headtail -k error,warning -i input.txt

# Case-insensitive keyword matching
headtail -K error,Warning -i input.txt

# Combine options
headtail -n 10 -s -K error -i input.txt
```