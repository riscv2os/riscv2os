// 檔案系統的五個層次，底下為 virtio，上面為系統呼叫，共七層
// File system implementation.  Five layers: 
//   + Blocks: allocator for raw disk blocks.
//   + Log: crash recovery for multi-step updates.
//   + Files: inode allocator, reading, writing, metadata.
//   + Directories: inode with special contents (list of other inodes!)
//   + Names: paths like /usr/rtm/xv6/fs.c for convenient naming.
//
// This file contains the low-level file system manipulation
// routines.  The (higher-level) system call implementations
// are in sysfile.c.
/*
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"
#include "file.h"
*/
#define min(a, b) ((a) < (b) ? (a) : (b))
// there should be one superblock per disk device, but we run with
// only one device
struct superblock sb; // 超級區塊 (包含硬碟中有多少區塊，inode，可用區塊等資訊)
FILE *fd;


// Init fs
void
fs_init(int dev) { // 檔案系統初始化
  fs_readsb(dev, &sb); // 讀入超級區塊
  if(sb.magic != FSMAGIC)
    panic("invalid file system");
  initlog(dev, &sb); // 初始化日誌 log 功能
}

// Read the super block.
static void
fs_readsb(int dev, struct superblock *sb) // 讀取超級區塊
{
  struct buf *bp;

  bp = buf_read(dev, 1); // 第一塊是超級區塊，呼叫 bread 讀入到 bp
  memmove(sb, bp->data, sizeof(*sb)); // 複製 bp 給 sb
  brelse(bp); // 釋放 bp
}
