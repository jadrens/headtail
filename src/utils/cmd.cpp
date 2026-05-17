#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

// Function to get console width
int get_console_width(void) {
    struct winsize w;
    
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
        return w.ws_col;
    }
    
    return 80; // Default width if unable to get
}

// Function to get console height
int get_console_height(void) {
    struct winsize w;
    
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
        return w.ws_row;
    }
    
    return 24; // Default height if unable to get
}

// Function to get complete console information
int get_console_size(struct winsize *w) {
    if (w == NULL) {
        return -1;
    }
    
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, w) != -1) {
        return 0; // Success
    }
    
    return -1; // Failure
}
