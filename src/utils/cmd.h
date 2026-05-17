#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

int get_console_width(void);
int get_console_height(void);
int get_console_size(struct winsize *w);