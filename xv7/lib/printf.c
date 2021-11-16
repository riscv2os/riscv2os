#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#include <stdarg.h>

static char digits[] = "0123456789ABCDEF";

static void
putc(int fd, char c)
{
  write(fd, &c, 1);
}

static int
printint(int fd, int xx, int base, int sgn)
{
  char buf[16];
  int i, neg;
  uint x;

  neg = 0;
  if(sgn && xx < 0){
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);
  if(neg)
    buf[i++] = '-';

  int count = i;
  while(--i >= 0)
    putc(fd, buf[i]);
  return count;
}

static int
printptr(int fd, uint64 x) {
  int i;
  putc(fd, '0');
  putc(fd, 'x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    putc(fd, digits[x >> (sizeof(uint64) * 8 - 4)]);
  return i+2;
}

// Print to the given fd. Only understands %d, %x, %p, %s.
int
vprintf(int fd, const char *fmt, va_list ap)
{
  char *s;
  int c, i, state, count=0;

  state = 0;
  for(i = 0; fmt[i]; i++){
    c = fmt[i] & 0xff;
    if(state == 0){
      if(c == '%'){
        state = '%';
      } else {
        putc(fd, c);
        count++;
      }
    } else if(state == '%'){
      if(c == 'd'){
        count += printint(fd, va_arg(ap, int), 10, 1);
      } else if(c == 'l') {
        count += printint(fd, va_arg(ap, uint64), 10, 0);
      } else if(c == 'x') {
        count += printint(fd, va_arg(ap, int), 16, 0);
      } else if(c == 'p') {
        count += printptr(fd, va_arg(ap, uint64));
      } else if(c == 's'){
        s = va_arg(ap, char*);
        if(s == 0)
          s = "(null)";
        while(*s != 0){
          putc(fd, *s);
          s++;
          count++;
        }
      } else if(c == 'c'){
        putc(fd, va_arg(ap, uint));
        count++;
      } else if(c == '%'){
        putc(fd, c);
        count++;
      } else {
        // Unknown % sequence.  Print it to draw attention.
        putc(fd, '%');
        putc(fd, c);
        count+=2;
      }
      state = 0;
    }
  }
  return count;
}

int
fprintf(int fd, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  return vprintf(fd, fmt, ap);
}

int
printf(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  return vprintf(1, fmt, ap);
}
