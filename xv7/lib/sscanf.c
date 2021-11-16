#include <stdarg.h>
#include "lib.h"

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

int vsscanf(const char *text, const char *fmt, va_list vl) {
 	const char *fp=fmt, *tp=text;
  char *s, *endp;;
  int ret=0;
 	while (fmt && *fp && *tp)	{
 	    if (*fp == '%') {
 	       fp++;
 	       switch (*fp) {
 	           case 's': 
               s = va_arg(vl, char*);
               while (*tp != ' ' && *tp != '\0') *s++=*tp++;
	 	           ret ++;
	 	           break;
 	           case 'c': 
	 	           *(char *)va_arg( vl, char* ) = *tp++;
	 	           ret ++;
	 	           break;
 	           case 'd': 
	 	           *(int *)va_arg( vl, int* ) =strtoi(tp, &endp, 10);
	 	           tp = endp;
	 	           ret++;
	 	           break;
 	           case 'x': 
	 	           *(int *)va_arg( vl, int* ) =strtoi(tp, &endp, 16);
	 	           tp =endp;
	 	           ret++;
	 	           break;
 	        }
 	    } 
 	    else {
        if (*tp != *fp) return ret;
        tp++;
      }
      fp++;
    }
    return ret;
}

int sscanf(const char *text, const char *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);
  return vsscanf(text, fmt, vl);
}
