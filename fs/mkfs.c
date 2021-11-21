#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "fs.h"

int main(int argc, char *argv[])
{
  assert(sizeof(int) == 4); // 整數必須是 4 bytes

  if(argc < 2){
    fprintf(stderr, "Usage: mkfs fs.img files...\n");
    exit(1);
  }

  fs_open(argv[1]); 
  fs_init();
  fs_save();
  fs_build(argc-2, &argv[2]);
}
