#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "kernel/memlayout.h"
#include "user/user.h"

#define NCHILD 2
#define N 100000

void test0();
void test1();

int
main(int argc, char *argv[])
{
  test0();
  test1();
  exit(0);
}

void test0()
{
  void *a, *a1;
  int n = 0;
  printf("start test0\n");  
  ntas(0);
  for(int i = 0; i < NCHILD; i++){
    int pid = fork();
    if(pid < 0){
      printf("fork failed");
      exit(-1);
    }
    if(pid == 0){
      for(i = 0; i < N; i++) {
        a = sbrk(4096);
        *(int *)(a+4) = 1;
        a1 = sbrk(-4096);
        if (a1 != a + 4096) {
          printf("wrong sbrk\n");
          exit(-1);
        }
      }
      exit(-1);
    }
  }

  for(int i = 0; i < NCHILD; i++){
    wait(0);
  }
  printf("test0 results:\n");
  n = ntas(1);
  if(n < 10) 
    printf("test0 OK\n");
  else
    printf("test0 FAIL\n");
}

// Run system out of memory and count tot memory allocated
void test1()
{
  void *a;
  int pipes[NCHILD];
  int tot = 0;
  char buf[1];
  
  printf("start test1\n");  
  for(int i = 0; i < NCHILD; i++){
    int fds[2];
    if(pipe(fds) != 0){
      printf("pipe() failed\n");
      exit(-1);
    }
    int pid = fork();
    if(pid < 0){
      printf("fork failed");
      exit(-1);
    }
    if(pid == 0){
      close(fds[0]);
      for(i = 0; i < N; i++) {
        a = sbrk(PGSIZE);
        *(int *)(a+4) = 1;
        if (write(fds[1], "x", 1) != 1) {
          printf("write failed");
          exit(-1);
        }
      }
      exit(0);
    } else {
      close(fds[1]);
      pipes[i] = fds[0];
    }
  }
  int stop = 0;
  while (!stop) {
    stop = 1;
    for(int i = 0; i < NCHILD; i++){
      if (read(pipes[i], buf, 1) == 1) {
        tot += 1;
        stop = 0;
      }
    }
  }
  int n = (PHYSTOP-KERNBASE)/PGSIZE;
  printf("total allocated number of pages: %d (out of %d)\n", tot, n);
  if(n - tot > 1000) {
    printf("test1 FAILED: cannot allocate enough memory");
    exit(-1);
  }
  printf("test1 OK\n");  
}

