#pragma once

#include "riscv.h"
#include <stddef.h>
#include <stdarg.h>

#define EOF 0

#define lib_error(...)       \
  {                          \
    lib_printf(__VA_ARGS__); \
    while (1)                \
    {                        \
    }                        \
  }                          \
  }

extern char *lib_gets(char *);
extern void uart_init();
extern char lib_kbhit();
extern int lib_getc(void);
extern void lib_delay(volatile int count);
extern int lib_putc(char ch);
extern void lib_puts(char *s);
extern int lib_printf(const char *s, ...);
extern int lib_vprintf(const char *s, va_list vl);
extern int lib_vsnprintf(char *out, size_t n, const char *s, va_list vl);

