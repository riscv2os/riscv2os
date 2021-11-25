#include "mylib.h"

static char digits[] = "0123456789ABCDEF";

static char* _printint(char *text, int xx, int base, int sgn) {
  char buf[16];
  int i, neg;
  unsigned int x;

  neg = 0;
  if(sgn && xx < 0){
  neg = 1;
  x = -xx;
  } else {
  x = xx;
  }

  i = 0;
  do {
  buf[i++] = digits[x % base];
  }while((x /= base) != 0);
  if(neg)
  buf[i++] = '-';

  char *p = text;
  while(--i >= 0)
  *p++ = buf[i];
  return p;
}

// Print to text. Only understands %d, %x, %s.
int _vsprintf(char *text, const char *fmt, va_list vl) {
  char *s, *p=text;
  int c, i, state;

  state = 0;
  for(i = 0; fmt[i]; i++){
    c = fmt[i];
    if(state == 0){
      if(c == '%'){
        state = '%';
      } else {
        *p++ = c;
      }
    } else if(state == '%'){
      if(c == 'd'){
        p=_printint(p, va_arg(vl, int), 10, 1);
      } else if(c == 'l') {
        p=_printint(p, va_arg(vl, long), 10, 0);
      } else if(c == 'x') {
        p=_printint(p, va_arg(vl, int), 16, 0);
      } else if(c == 's'){
        s = va_arg(vl, char*);
        if(s == 0)
          s = "(null)";
        while(*s != 0) {
          *p++ = *s++;
        }
      } else if(c == 'c'){
        *p++ = va_arg(vl, unsigned int);
      } else if(c == '%'){
        *p++ = c;
      } else { // Unknown % sequence.  Print it to draw attention.
        *p++ = '%';
        *p++ = c;
      }
      state = 0;
    }
  }
  *p = '\0';
  return p-text;
}

int _sprintf(char *text, const char *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);
  int len = _vsprintf(text, fmt, vl);
  return len;
}

int _fprintf(FILE *stream, const char *fmt, ...) {
  char text[1024];
  va_list vl;
  va_start(vl, fmt);
  int len = _vsprintf(text, fmt, vl);
  return fputs(text, stream);
  // return len;
}

int _printf(const char *fmt, ...) {
  char text[1024];
  va_list vl;
  va_start(vl, fmt);
  int len = _vsprintf(text, fmt, vl);
  return fputs(text, stdout);
}
