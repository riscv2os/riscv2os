#pragma once

#include "stddef.h"
#include <vfs.h>

extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE* stream);
extern size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE* stream);
int fgetc(FILE *stream);
int fputc(int ch, FILE *stream);
int putchar(int ch);
int getchar(void);
int fputs(const char *s, FILE *stream);
int puts(const char *s);
char *fgets(char *s, int n, FILE *stream);
char *gets(char *s);
int vsprintf(char *text, const char *fmt, va_list ap);
int sprintf(char *text, const char *fmt, ...);
int printf(const char *fmt, ...);
int sscanf(const char *text, const char *fmt, ...);
