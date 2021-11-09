#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define stat xv6_stat  // avoid clash with host struct stat
#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "kernel/param.h"

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


void balloc(int);
void wsect(uint, void*);
void winode(uint, struct dinode*);
void rinode(uint inum, struct dinode *ip);
void rsect(uint sec, void *buf);
uint ialloc(ushort type);
void iappend(uint inum, void *p, int n);

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

void
wsect(uint sec, void *buf) // 寫入 buf 到第 sec 個磁區
{
  if(fseek(fsfd, sec * BSIZE, SEEK_SET)!=0){ // ccc: if(lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE){
    perror("lseek");
    exit(1);
  }
  if(fwrite(buf, 1, BSIZE, fsfd) != BSIZE){ // ccc: if(write(fsfd, buf, BSIZE) != BSIZE){
    perror("write");
    exit(1);
  }
}

void
winode(uint inum, struct dinode *ip) // 寫入第 inum 個 inode
{
  char buf[BSIZE];
  uint bn;
  struct dinode *dip;

  bn = IBLOCK(inum, sb);
  rsect(bn, buf);
  dip = ((struct dinode*)buf) + (inum % IPB);
  *dip = *ip;
  wsect(bn, buf);
}

void
rinode(uint inum, struct dinode *ip) // 讀取第 inum 個 inode
{
  char buf[BSIZE];
  uint bn;
  struct dinode *dip;

  bn = IBLOCK(inum, sb);
  rsect(bn, buf);
  dip = ((struct dinode*)buf) + (inum % IPB);
  *ip = *dip;
}

void
rsect(uint sec, void *buf) // 讀取第 sec 個磁區到 buf 中
{
  if(fseek(fsfd, sec * BSIZE, SEEK_SET)!=0){ // ccc: if(lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE){
    perror("lseek");
    exit(1);
  }
  if(fread(buf, 1, BSIZE, fsfd) != BSIZE){ // ccc:if(read(fsfd, buf, BSIZE) != BSIZE){
    perror("read");
    printf("rsect error: read size != BSIZE\n");
    exit(1);
  }
}

uint
ialloc(ushort type) // 分配一個型態為 type 的 inode
{
  uint inum = freeinode++; // 取得一個沒人用的 inode 代號
  struct dinode din; // din 磁碟中的 inode

  bzero1(&din, sizeof(din));
  din.type = xshort(type);   // 檔案型態
  din.nlink = xshort(1);     // 連結數為 1
  din.size = xint(0);        // 目前 inode 對應的檔案大小為 0
  winode(inum, &din);        // 寫入
  return inum;
}

void
balloc(int used) // 分配 used 個資料區塊
{
  uchar buf[BSIZE];
  int i;

  printf("balloc: first %d blocks have been allocated\n", used);
  assert(used < BSIZE*8);
  bzero1(buf, BSIZE);
  for(i = 0; i < used; i++){
    buf[i/8] = buf[i/8] | (0x1 << (i%8));
  }
  printf("balloc: write bitmap block at sector %d\n", sb.bmapstart);
  wsect(sb.bmapstart, buf); // 將這些修改過的 bitmap 寫回磁碟
}

#define min(a, b) ((a) < (b) ? (a) : (b))

void
iappend(uint inum, void *xp, int n) // 新增 xp[0..n] 這些資料到第 inum 號 inode 中
{
  char *p = (char*)xp;
  uint fbn, off, n1;
  struct dinode din;
  char buf[BSIZE];
  uint indirect[NINDIRECT];
  uint x;

  rinode(inum, &din);   // 讀取該 inode
  off = xint(din.size); // off 指到尾端
  // printf("append inum %d at off %d sz %d\n", inum, off, n);
  while(n > 0){
    fbn = off / BSIZE;     // 取得區塊號
    assert(fbn < MAXFILE); // 不可超出最大檔案大小
    if(fbn < NDIRECT){     // 直接區塊
      if(xint(din.addrs[fbn]) == 0){ // 若該 addrs[fbn] 是空的
        din.addrs[fbn] = xint(freeblock++); // 分配一個自由區塊，append 進 inode
      }
      x = xint(din.addrs[fbn]); // 取得 addrs 索引 x
    } else {               // 間接區塊
      if(xint(din.addrs[NDIRECT]) == 0){ // 若間接索引 addrs[NDIRECT] 是空的
        din.addrs[NDIRECT] = xint(freeblock++); // 分配一個自由區塊，append 進 inode
      }
      rsect(xint(din.addrs[NDIRECT]), (char*)indirect); // 讀進該間接索引區塊
      if(indirect[fbn - NDIRECT] == 0){ // 若那一格還沒區塊
        indirect[fbn - NDIRECT] = xint(freeblock++); // 分配一個區塊
        wsect(xint(din.addrs[NDIRECT]), (char*)indirect); // 寫入該間接索引區塊
      }
      x = xint(indirect[fbn-NDIRECT]); // 取得 addrs 索引 x
    }
    n1 = min(n, (fbn + 1) * BSIZE - off); // 計算寫入了多少 bytes
    // 然後真正把這些 byte 寫入
    rsect(x, buf);
    memmove(buf + off - (fbn * BSIZE), p, n1);  // ccc: bcopy(p, buf + off - (fbn * BSIZE), n1);
    wsect(x, buf);
    n -= n1;    // 扣掉已寫入的大小
    off += n1;  // 向前邁進
    p += n1;
  }
  din.size = xint(off);  // 設定目前 inode 大小
  winode(inum, &din);    // 將該 inode 寫回
}
