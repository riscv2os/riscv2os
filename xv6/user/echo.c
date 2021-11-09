#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;

  for(i = 1; i < argc; i++){ // 對於每個參數
    write(1, argv[i], strlen(argv[i])); // 都印到 fd=1 的 CONSOLE
    if(i + 1 < argc){
      write(1, " ", 1); // 中間用空格隔開
    } else {
      write(1, "\n", 1); // 最後要換行
    }
  }
  exit(0);
}
