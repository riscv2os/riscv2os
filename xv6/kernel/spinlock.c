// Mutual exclusion spin locks. 
// 旋轉鎖，透過禁止中斷與 hartid 檢查，避免兩個核心同時存取某變數的機制

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"

void
initlock(struct spinlock *lk, char *name) // 設定一個鎖
{
  lk->name = name; // 鎖的名稱
  lk->locked = 0;  // 初始值為未上鎖
  lk->cpu = 0;     // 哪個處理器鎖的？
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
void
acquire(struct spinlock *lk)
{
  push_off(); // disable interrupts to avoid deadlock. // 計數版禁止中斷
  if(holding(lk)) // 如果同一處理器重複鎖定，那就是嚴重錯誤
    panic("acquire");

  // On RISC-V, sync_lock_test_and_set turns into an atomic swap:
  //   a5 = 1
  //   s1 = &lk->locked
  //   amoswap.w.aq a5, a5, (s1)
  while(__sync_lock_test_and_set(&lk->locked, 1) != 0) // 等待直到鎖定成功
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen strictly after the lock is acquired.
  // On RISC-V, this emits a fence instruction.
  __sync_synchronize(); // 要求編譯器載入儲存指令不跨越此邊界

  // Record info about lock acquisition for holding() and debugging.
  lk->cpu = mycpu(); // 設定上鎖者的處理器代號
}

// Release the lock.
void
release(struct spinlock *lk)
{
  if(!holding(lk)) // 如果沒有鎖定就呼叫 release()，那就是嚴重錯誤
    panic("release");

  lk->cpu = 0; // 釋放後處理器值還原為 0

  // Tell the C compiler and the CPU to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other CPUs before the lock is released,
  // and that loads in the critical section occur strictly before
  // the lock is released.
  // On RISC-V, this emits a fence instruction.
  __sync_synchronize(); // 要求編譯器載入儲存指令不跨越此邊界

  // Release the lock, equivalent to lk->locked = 0.
  // This code doesn't use a C assignment, since the C standard
  // implies that an assignment might be implemented with
  // multiple store instructions.
  // On RISC-V, sync_lock_release turns into an atomic swap:
  //   s1 = &lk->locked
  //   amoswap.w zero, zero, (s1)
  __sync_lock_release(&lk->locked); // 解除鎖定

  pop_off(); // 允許中斷
}

// Check whether this cpu is holding the lock.
// Interrupts must be off.
int
holding(struct spinlock *lk) // 檢查本處理器是否鎖定了 lk
{
  int r;
  r = (lk->locked && lk->cpu == mycpu());
  return r;
}

// push_off/pop_off are like intr_off()/intr_on() except that they are matched:
// it takes two pop_off()s to undo two push_off()s.  Also, if interrupts
// are initially off, then push_off, pop_off leaves them off.

void
push_off(void) // 計數版禁止中斷
{
  int old = intr_get(); // 取得目前中斷狀態

  intr_off(); // 禁止中斷
  if(mycpu()->noff == 0)   // 若計數為 0
    mycpu()->intena = old; // mycpu()->intena = 剛剛取得的中斷狀態
  mycpu()->noff += 1; // 將計數加一
}

void
pop_off(void) // 計數版允許中斷
{
  struct cpu *c = mycpu();
  if(intr_get())
    panic("pop_off - interruptible");
  if(c->noff < 1)
    panic("pop_off");
  c->noff -= 1; // 將計數減一
  if(c->noff == 0 && c->intena) // 若計數為 0 且原本為禁止中斷，則允許中斷
    intr_on();
}
