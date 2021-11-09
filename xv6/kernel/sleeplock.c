// Sleeping locks // 睡眠鎖，等待某 spinlock 完成，暫時進入 sleep 狀態

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

void
initsleeplock(struct sleeplock *lk, char *name) // sleeplock 的初始化函數
{
  initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
}

void
acquiresleep(struct sleeplock *lk) // 取得睡眠鎖
{
  acquire(&lk->lk); // 試圖鎖定
  while (lk->locked) {  // 若已經被鎖定
    sleep(lk, &lk->lk); // 沉睡直到鎖定解除
  }
  lk->locked = 1; // 鎖定
  lk->pid = myproc()->pid; // 紀錄鎖定的行程代號
  release(&lk->lk); // 釋放該鎖
}

void
releasesleep(struct sleeplock *lk) // 釋放睡眠鎖
{
  acquire(&lk->lk);
  lk->locked = 0;
  lk->pid = 0;
  wakeup(lk); // 喚醒等待此鎖的行程。
  release(&lk->lk);
}

int
holdingsleep(struct sleeplock *lk) // 檢查本行程是否鎖定了 lk
{
  int r;
  
  acquire(&lk->lk);
  r = lk->locked && (lk->pid == myproc()->pid);
  release(&lk->lk);
  return r;
}



