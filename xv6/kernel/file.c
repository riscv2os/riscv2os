// 檔案表與讀寫的相關操作
// Support functions for system calls that involve file descriptors.
//

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "stat.h"
#include "proc.h"

struct devsw devsw[NDEV]; // 裝置表
struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable; // 檔案表

void
fileinit(void)
{
  initlock(&ftable.lock, "ftable"); // 此模組的鎖
}

// Allocate a file structure. (open 時會找出 ftable 中哪格可以用)
struct file*
filealloc(void)
{
  struct file *f;

  acquire(&ftable.lock);
  for(f = ftable.file; f < ftable.file + NFILE; f++){ // 對於整個檔案表
    if(f->ref == 0){                                  // 找到一格沒被使用的
      f->ref = 1;                                     // 設定成已被使用
      release(&ftable.lock);
      return f;                                       // 傳回該檔案描述子
    }
  }
  release(&ftable.lock);
  return 0;
}

// Increment ref count for file f. (增加 f 檔案描述子的引用次數)
struct file*
filedup(struct file *f)
{
  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("filedup");
  f->ref++;
  release(&ftable.lock);
  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f) // 關閉檔案
{
  struct file ff;

  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("fileclose");
  if(--f->ref > 0){ // 若還有人用，就直接返回
    release(&ftable.lock);
    return;
  }
  // 已經沒人用了，回收對應的那一格
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  release(&ftable.lock);

  if(ff.type == FD_PIPE){
    pipeclose(ff.pipe, ff.writable);
  } else if(ff.type == FD_INODE || ff.type == FD_DEVICE){ // 若是真的檔案 (非 pipe 或 console ...)
    begin_op();
    iput(ff.ip); // 將緩衝內容寫出去
    end_op();
  }
}

// Get metadata about file f. (取得檔案資訊)
// addr is a user virtual address, pointing to a struct stat. (addr 是使用者空間的位址，指向 struct stat)
int
filestat(struct file *f, uint64 addr)
{
  struct proc *p = myproc(); // p = 本行程
  struct stat st; // 檔案資訊
  
  if(f->type == FD_INODE || f->type == FD_DEVICE){ // 若是 inode 或裝置
    ilock(f->ip);
    stati(f->ip, &st); // 取得 inode 內的資訊
    iunlock(f->ip);
    if(copyout(p->pagetable, addr, (char *)&st, sizeof(st)) < 0) // 複製到使用者空間
      return -1;
    return 0;
  }
  return -1;
}

// Read from file f.
// addr is a user virtual address.
int
fileread(struct file *f, uint64 addr, int n) // 檔案讀取
{
  int r = 0;

  if(f->readable == 0) // 若非可讀，離開
    return -1;

  if(f->type == FD_PIPE){ // 若是管道
    r = piperead(f->pipe, addr, n); // 呼叫 piperead
  } else if(f->type == FD_DEVICE){ // 若是裝置
    if(f->major < 0 || f->major >= NDEV || !devsw[f->major].read)
      return -1;
    r = devsw[f->major].read(1, addr, n); // 呼叫裝置 read 函數
  } else if(f->type == FD_INODE){ // 若是磁碟 inode
    ilock(f->ip);
    if((r = readi(f->ip, 1, addr, f->off, n)) > 0) // 呼叫 readi
      f->off += r;
    iunlock(f->ip);
  } else {
    panic("fileread");
  }

  return r;
}

// Write to file f.
// addr is a user virtual address.
int
filewrite(struct file *f, uint64 addr, int n) // 檔案寫入
{
  int r, ret = 0;

  if(f->writable == 0) // 若非可寫，離開
    return -1;

  if(f->type == FD_PIPE){ // 若是管道
    ret = pipewrite(f->pipe, addr, n); // 呼叫 pipewrite
  } else if(f->type == FD_DEVICE){ // 若是裝置
    if(f->major < 0 || f->major >= NDEV || !devsw[f->major].write)
      return -1;
    ret = devsw[f->major].write(1, addr, n); // 呼叫裝置 write函數
  } else if(f->type == FD_INODE){ // 若是 inode
    // write a few blocks at a time to avoid exceeding    一次寫很多塊
    // the maximum log transaction size, including        降低 log 大小
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    // this really belongs lower down, since writei()
    // might be writing a device like the console.
    int max = ((MAXOPBLOCKS-1-1-2) / 2) * BSIZE;
    int i = 0;
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;

      begin_op();
      ilock(f->ip);
      if ((r = writei(f->ip, 1, addr + i, f->off, n1)) > 0) // 呼叫 writei 寫入到磁碟
        f->off += r;
      iunlock(f->ip);
      end_op();

      if(r != n1){
        // error from writei
        break;
      }
      i += r;
    }
    ret = (i == n ? n : -1);
  } else {
    panic("filewrite");
  }

  return ret;
}

