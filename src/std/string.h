#pragma once

#include <stddef.h>

char* strcpy(char *s, const char *t);
int strcmp(const char *p, const char *q);
size_t strlen(const char *s);
char* strchr(const char *s, int c);
void* memset(void *dst, int c, size_t n);
void* memmove(void *vdst, const void *vsrc, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
int strtoi(const char *s, char **end, int base);

