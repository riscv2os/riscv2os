#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "types.h"

// On-disk file system format. // 磁碟中的檔案系統結構
// Both the kernel and user programs use this header file.
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       1000  // size of file system in blocks
#define MAXPATH      128   // maximum file path name

#define ROOTINO  1   // root i-number 根結點
#define BSIZE 1024   // block size    區塊大小    

// Disk layout:
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock { // 超級區塊
  uint magic;        // Must be FSMAGIC      // 用來辨識的魔數:0x10203040
  uint size;         // Size of file system image (blocks) // 全部區塊數
  uint nblocks;      // Number of data blocks // 資料區塊數
  uint ninodes;      // Number of inodes.     // inodes 數量
  uint nlog;         // Number of log blocks  // 日誌 log 的區塊數
  uint logstart;     // Block number of first log block   // log 的首區塊
  uint inodestart;   // Block number of first inode block // inode 的首區塊
  uint bmapstart;    // Block number of first free map block // free bitmap 的首區塊
};

#define FSMAGIC 0x10203040

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

#define major(dev)  ((dev) >> 16 & 0xFFFF) // 取得裝置主序號
#define minor(dev)  ((dev) & 0xFFFF)       // 取得裝置次序號
#define	mkdev(m,n)  ((uint)((m)<<16| (n))) // 主序號+次序號=完整序號

// in-memory copy of an inode (記憶體內的 inode)
struct inode {
  uint dev;           // Device number (裝置號碼)
  uint inum;          // Inode number (inode 號碼)
  int ref;            // Reference count (引用次數)
  // struct sleeplock lock; // protects everything below here (鎖)
  int valid;          // inode has been read from disk? (是否讀進來了)

  short type;         // copy of disk inode (inode 型態)
  short major;        // 主序號
  short minor;        // 次序號
  short nlink;        // 連結次數
  uint size;          // 檔案大小
  uint addrs[NDIRECT+1]; // 區塊位址
};

// On-disk inode structure (硬碟中的 inode 結構)
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEVICE only)
  short minor;          // Minor device number (T_DEVICE only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};

// Inodes per block. (每個區塊的 inode 數量)
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i (第 i 個 inode 存放在哪個區塊)
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block (每個區塊包含幾個 bits)
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b (取得 free map 中紀錄該區塊是否 free 的區塊代號)
#define BBLOCK(b, sb) ((b)/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent { // 目錄中的一項 (inode 代號+名稱)。
  ushort inum;
  char name[DIRSIZ];
};

struct buf { // 緩衝區塊
  int valid;    // has data been read from disk? // 已讀入？
  int disk;     // does disk "own" buf? // 正在讀？
  uint dev;     // 裝置代號
  uint blockno; // 區塊代號
  // struct sleeplock lock; // 鎖定中？
  uint refcnt;      // 引用數量
  struct buf *prev; // LRU cache list
  struct buf *next; // 緩衝區塊串列
  uchar data[BSIZE]; // 資料內容
};

// ============ stat.h =================
#define T_DIR     1   // Directory
#define T_FILE    2   // File
#define T_DEVICE  3   // Device

struct stat { // 狀態資訊，可能是《目錄，檔案或裝置》
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short type;  // Type of file
  short nlink; // Number of links to file
  uint64 size; // Size of file in bytes
};


// ============== fs api ===============
void fs_open(char *imgFile);
int fs_init();
int fs_save();
int fs_build(int fcount, char *fnames[]);
