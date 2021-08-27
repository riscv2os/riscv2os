#include <string.h>

char* strcpy(char *s, const char *t) {
  char *os;
  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int strcmp(const char *p, const char *q) {
  while(*p && *p == *q)
    p++, q++;
  // return (uchar)*p - (uchar)*q;
  return *p - *q;
}

size_t strlen(const char *s) {
  int n;
  for(n = 0; s[n]; n++)
    ;
  return n;
}

char* strchr(const char *s, int c) {
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

// ================== memory =============================
void* memset(void *dst, int c, size_t n) {
  char *cdst = (char *) dst;
  int i;
  for(i = 0; i < n; i++){
    cdst[i] = c;
  }
  return dst;
}

void* memmove(void *vdst, const void *vsrc, size_t n) {
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  if (src > dst) {
    while(n-- > 0)
      *dst++ = *src++;
  } else {
    dst += n;
    src += n;
    while(n-- > 0)
      *--dst = *--src;
  }
  return vdst;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const char *p1 = s1, *p2 = s2;
  while (n-- > 0) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
    p1++;
    p2++;
  }
  return 0;
}

void *memcpy(void *dst, const void *src, size_t n) {
  return memmove(dst, src, n);
}

// ========================= integer/string convert ===============
static char digits[] = "0123456789ABCDEF";

int strtoi(const char *s, char **end, int base) {
  const char *p = s, *dp;
  int n=0;
  while((dp = strchr(digits, *p))!=0) {
    n = n*base + (dp-digits);
    p++;
  }
  *end = (char *)p;
  return n;
}

/*
int atoi(const char *s) {
  int n;
  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}
*/
