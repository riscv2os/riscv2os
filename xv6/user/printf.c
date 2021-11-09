#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#include <stdarg.h>

static char digits[] = "0123456789ABCDEF";

static void
putc(int fd, char c) // 印出字元
{
  write(fd, &c, 1);
}

static void
printint(int fd, int xx, int base, int sgn) // 印出整數
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

  while(--i >= 0)
    putc(fd, buf[i]);
}

static void
printptr(int fd, uint64 x) { // 印出指標
  int i;
  putc(fd, '0');
  putc(fd, 'x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    putc(fd, digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the given fd. Only understands %d, %x, %p, %s.
void
vprintf(int fd, const char *fmt, va_list ap) // 將 ap 中的參數填入 fmt 後印到檔案 fd 中。
{
  char *s;
  int c, i, state;

  state = 0;
  for(i = 0; fmt[i]; i++){
    c = fmt[i] & 0xff;
    if(state == 0){
      if(c == '%'){
        state = '%';
      } else {
        putc(fd, c);
      }
    } else if(state == '%'){
      if(c == 'd'){ // %d : 整數
        printint(fd, va_arg(ap, int), 10, 1);
      } else if(c == 'l') { // %l : 長整數
        printint(fd, va_arg(ap, uint64), 10, 0);
      } else if(c == 'x') { // %x : 16 進位整數
        printint(fd, va_arg(ap, int), 16, 0);
      } else if(c == 'p') { // %p : 指標
        printptr(fd, va_arg(ap, uint64));
      } else if(c == 's'){
        s = va_arg(ap, char*);
        if(s == 0)
          s = "(null)";
        while(*s != 0){
          putc(fd, *s);
          s++;
        }
      } else if(c == 'c'){ // %c 字元
        putc(fd, va_arg(ap, uint));
      } else if(c == '%'){ // %% : 印出 %
        putc(fd, c);
      } else { // %? : 印出 %?  (? 是某字元，但該字元不是 x,p,n)
        // Unknown % sequence.  Print it to draw attention.
        putc(fd, '%');
        putc(fd, c);
      }
      state = 0;
    }
  }
}

void
fprintf(int fd, const char *fmt, ...) // 將參數透過 fmt 格式化後印出到 fd 檔案中
{
  va_list ap;

  va_start(ap, fmt);
  vprintf(fd, fmt, ap);
}

void
printf(const char *fmt, ...) // 將參數透過 fmt 格式化後印出到 stdout (fd=1)
{
  va_list ap;

  va_start(ap, fmt);
  vprintf(1, fmt, ap);
}
