#pragma once

#include <sys/ioctl.h>
#include <unistd.h>

int get_console_width(void);
int get_console_height(void);
int get_console_size(struct winsize *w);