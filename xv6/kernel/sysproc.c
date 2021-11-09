#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

// 行程相關的系統呼叫

uint64
sys_exit(void) // 結束行程 exit(n)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void) // 取得行程代號 getpid()
{
  return myproc()->pid;
}

uint64
sys_fork(void) // 複製行程 fork()
{
  return fork();
}

uint64
sys_wait(void) // 等待行程結束 wait(status)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void) // sbrk(n) 增加 n bytes 的行程空間 -- https://linux.die.net/man/2/sbrk 
// sbrk 名稱為改變 program break 的意思！ (例如 malloc 通常就會呼叫 sbrk 增加大小)
// https://ithelp.ithome.com.tw/articles/10186995
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void) // 睡眠 n 個 ticks， sleep(20) 為睡 20 個 ticks (xv6 中約兩秒)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void) // 刪除 pid 行程 kill(pid)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void) // 取得時間 (tick 數，一個 tick 是一次時間中斷的長度)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
