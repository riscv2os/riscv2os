#include "../lib/lib.h"

char buf[512];

void cp(char *fname1, char *fname2) {
  int fd1 = open(fname1, O_RDONLY);
  int fd2 = open(fname2, O_CREATE|O_WRONLY);
  int n;
  while((n = read(fd1, buf, sizeof(buf))) > 0){
    write(fd2, buf, n);
  }
  close(fd1);
  close(fd2);
}

int main(int argc, char *argv[]) {
  cp(argv[1], argv[2]);
  // exit(0);
  return 0;
}

