#pragma once

#include <stdio.h>
#include <stdarg.h>

#define _assert(cond) if (!cond) printf("Assertion failed!\nFile: %s, Line %d\n", __FILE__, __LINE__);

int _printf(const char *fmt, ...);
