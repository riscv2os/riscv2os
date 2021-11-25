#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define stat xv6_stat  // avoid clash with host struct stat
#include "types.h"
#include "fs.h"
#include "stat.h"
#include "param.h"

#ifndef static_assert
#define static_assert(a, b) do { switch (0) case 0: case (a): ; } while (0)
#endif

#define bzero1(buf, size) memset(buf, 0, size)

#define NINODES 200

// Disk layout:
// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]
// 磁碟布局
// [ boot 區塊  | 超級區塊 | 日誌 | inode 區塊   | free bit map | 資料區塊     ]

int nbitmap = FSSIZE/(BSIZE*8) + 1; // bitmap 大小
int ninodeblocks = NINODES / IPB + 1; // inode 區塊數量
int nlog = LOGSIZE; // log 數量
int nmeta;    // Number of meta blocks (boot, sb, nlog, inode, bitmap) // 非資料區塊數量 (描述區塊)
int nblocks;  // Number of data blocks // 資料區塊數量

FILE *fsfd; // int fsfd;
struct superblock sb; // 超級區塊
char zeroes[BSIZE]; // 
uint freeinode = 1;
uint freeblock; // 自由區塊的起頭


// convert to intel byte order
ushort
xshort(ushort x) // short 排列正規化
{
  ushort y;
  uchar *a = (uchar*)&y;
  a[0] = x;
  a[1] = x >> 8;
  return y;
}

uint
xint(uint x) // int 排列正規化
{
  uint y;
  uchar *a = (uchar*)&y;
  a[0] = x;
  a[1] = x >> 8;
  a[2] = x >> 16;
  a[3] = x >> 24;
  return y;
}

int
main(int argc, char *argv[])
{
  int i, cc; FILE *fd; // ccc: int i, cc, fd;
  uint rootino, inum, off;
  struct dirent de;
  char buf[BSIZE];
  struct dinode din;


  static_assert(sizeof(int) == 4, "Integers must be 4 bytes!"); // 整數必須是 4 bytes

  if(argc < 2){
    fprintf(stderr, "Usage: mkfs fs.img files...\n");
    exit(1);
  }

  assert((BSIZE % sizeof(struct dinode)) == 0); // 區塊大小必須是 dinode 大小的倍數
  assert((BSIZE % sizeof(struct dirent)) == 0); // 區塊大小必須是 dirent 大小的倍數

  fsfd = fopen(argv[1], "wb+"); // ccc: fsfd = open(argv[1], O_RDWR|O_CREAT|O_TRUNC, 0666);
  if(fsfd < 0){
    perror(argv[1]);
    exit(1);
  }

  // 1 fs block = 1 disk sector
  nmeta = 2 + nlog + ninodeblocks + nbitmap; // 非資料區塊數量, 前面加 2 是因為有啟動區塊和超級區塊
  nblocks = FSSIZE - nmeta; // 資料區塊數量

  // 超級區塊設定
  sb.magic = FSMAGIC;             // 魔數
  sb.size = xint(FSSIZE);         // 大小
  sb.nblocks = xint(nblocks);     // 資料區塊數
  sb.ninodes = xint(NINODES);     // inode 數量
  sb.nlog = xint(nlog);           // log 數量
  sb.logstart = xint(2);          // log 起始點
  sb.inodestart = xint(2+nlog);   // inode 起始點
  sb.bmapstart = xint(2+nlog+ninodeblocks); // bmap 起始點

  printf("nmeta %d (boot, super, log blocks %u inode blocks %u, bitmap blocks %u) blocks %d total %d\n",
         nmeta, nlog, ninodeblocks, nbitmap, nblocks, FSSIZE);

  freeblock = nmeta;     // the first free block that we can allocate (資料區塊的第一塊為目前的 freeblock)

  for(i = 0; i < FSSIZE; i++)
    wsect(i, zeroes);

  // 寫入超級區塊到第一區塊中
  memset(buf, 0, sizeof(buf));
  memmove(buf, &sb, sizeof(sb)); 
  wsect(1, buf);

  // 創建根目錄的 inode
  rootino = ialloc(T_DIR);
  assert(rootino == ROOTINO);

  // 根目錄下的 . 指向自身
  bzero1(&de, sizeof(de));
  de.inum = xshort(rootino);
  strcpy(de.name, ".");
  iappend(rootino, &de, sizeof(de)); // 根目錄指向 de

  // 根目錄下的 .. 指向自身
  bzero1(&de, sizeof(de));
  de.inum = xshort(rootino);
  strcpy(de.name, "..");
  iappend(rootino, &de, sizeof(de));

  for(i = 2; i < argc; i++){ // 對於參數列中的每個檔案，都加入到系統根目錄中
    // get rid of "user/"
    char *shortname;
    if(strncmp(argv[i], "user/", 5) == 0) // 只取 user/ 下檔案的檔名
      shortname = argv[i] + 5;
    else
      shortname = argv[i];
    
    assert(strchr(shortname, '/') == 0); // assert(index(shortname, '/') == 0); 

    if((fd = fopen(argv[i], "rb+")) == NULL){ // ccc: if((fd = open(argv[i], 0)) < 0){
      perror(argv[i]);
      exit(1);
    }

    // Skip leading _ in name when writing to file system.
    // The binaries are named _rm, _cat, etc. to keep the
    // build operating system from trying to execute them
    // in place of system binaries like rm and cat.
    if(shortname[0] == '_') // xv6 的執行檔以 _ 開頭，去掉 _ 才放入資料夾內
      shortname += 1;

    // 創建加入檔的 inode
    inum = ialloc(T_FILE);
    // 創建檔名等資訊
    bzero1(&de, sizeof(de));
    de.inum = xshort(inum);
    strncpy(de.name, shortname, DIRSIZ);
    iappend(rootino, &de, sizeof(de));
    // 然後將檔案內容一塊一塊放進系統
    while((cc = fread(buf, 1, sizeof(buf), fd)) > 0) // ccc: while((cc = read(fd, buf, sizeof(buf))) > 0)
      iappend(inum, buf, cc);
    fclose(fd); // close(fd);
  }

  // fix size of root inode dir (修正根目錄的 inode 大小)
  rinode(rootino, &din);
  off = xint(din.size);
  off = ((off/BSIZE) + 1) * BSIZE;
  din.size = xint(off);
  winode(rootino, &din);

  balloc(freeblock);

  exit(0);
}
